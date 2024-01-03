//#define PROFILER_ENABLED

#if defined(PROFILER_ENABLED) && defined(IMGUI_DISABLE)
#warning "PROFILER_ENABLED is defined but so is IMGUI_DISABLE; ignoring PROFILER_ENABLED!"
#undef PROFILER_ENABLED
#endif

#ifdef PROFILER_ENABLED

u32 const MAX_DEBUG_EVENTS = 64 * 1024 * 128;
u32 const MAX_FRAME_PROFILES = 60;
u32 const MAX_BLOCK_PROFILES = 1024 * 256;

// TODO: Eventually we'd like to expand the profiler to support
// anciliary data being provided alongside the TIMED_FUNCTION()s
// and TIMED_BLOCK("")s. We could then make that data available
// in the visualizations.
enum Debug_Event_Type : u8 {
    DEBUG_EVENT_BLOCK_START,
    DEBUG_EVENT_BLOCK_END
};

#pragma pack(push, 1)
struct Debug_Event {
    Debug_Event_Type type; // 1 byte
    cstr guid;             // 8 bytes
    cstr name;             // 8 bytes
    cstr file;             // 8 bytes
    u32 line;              // 4 bytes
    u64 time;              // 8 bytes
    u8 reserved[27];       // 8+8+8+8+4+1 = 37 + 27 = 64 = 1 cache line
};
#pragma pack(pop)
static_assert(sizeof(Debug_Event) == 64);

s32 _cmp_block_profiles(void const* _a, void const* _b);

struct GUID {
    union {
        meow_u128 u128;
        struct {
            u64 lo;
            u64 hi;
        };
    };

    GUID() : lo(0), hi(0) {}
    GUID(meow_u128 _u128) : u128(_u128) {}
    GUID(u64 _lo, u64 _hi) : lo(_lo), hi(_hi) {}

    bool operator==(GUID const& b) const { return MeowHashesAreEqual(u128, b.u128); }
    bool operator!=(GUID const& b) const { return !MeowHashesAreEqual(u128, b.u128); }
};

bool guid_eq_fn(GUID const& a, GUID const& b) { return a == b; }


struct Block_Profile : Pooled<Block_Profile> {
    struct Profiler *p;
    GUID guid;
    cstr name;
    cstr file;
    u32 line;

    u32 count;
    u64 time;
    u64 child_time;

    Block_Profile* prev;
    Block_Profile* children;

    Block_Profile(struct Profiler *_p, GUID _guid, cstr _name, cstr _file, u32 _line) : p(_p), guid(_guid), name(_name), file(_file), line(_line), count(0), time(0), child_time(0), prev(NULL), children(NULL) {
    }

    template<typename T>
    void deinit(void (T::*return_pooled)(Block_Profile*)) {
        auto child = children;
        while(child) {
            auto tmp = child->prev;
            child->deinit(return_pooled);
            (p->*return_pooled)(child);
            child = tmp;
        }
        assert(!child);
        children = NULL;
    }

    void calculate_child_time() {
        child_time = 0;
        for(auto child = children; child != NULL; child = child->prev) {
            child->calculate_child_time();
            child_time += child->time;
        }
    }

    void sort() {
        // NOTE: See comment in Frame_Profile::sort

        //for(auto child : children) {
        //    child->sort();
        //}
        //qsort(children.data, children.count, sizeof(Block_Profile*), _cmp_block_profiles);
    }
};

s32 _cmp_block_profiles(void const* _a, void const* _b) {
    auto a = cast(Block_Profile const*, _a);
    auto b = cast(Block_Profile const*, _b);
    if(a->time < b->time) return 1;
    if(a->time > b->time) return -1;
    return 0;
}

struct Frame_Profile {
    struct Profiler *p;
    u64 frame;
    Block_Profile *block_profiles = NULL;
    u64 debug_events;
    u64 total_block_profiles;

    void deinit() {
        // TODO
    }

    template<typename T>
    void clear(void (T::*return_pooled)(Block_Profile*)) {
        // NOTE TODO: Deduplicate this (Block_Profile::deinit)

        auto child = block_profiles;
        while(child) {
            auto tmp = child->prev;
            child->deinit(return_pooled);
            (p->*return_pooled)(child);
            child = tmp;
        }
        assert(!child);
        block_profiles = NULL;
    }

    void sort_block_profiles() {
        // TODO: We need to implement merge sort for linked lists
        // in order to reimplement the sorting of Block_Profiles.
        // Apparently merge sort is O(n*log(n)) for both
        // the average _and_ worst case for sorting linked lists.
        // https://www.chiark.greenend.org.uk/~sgtatham/algorithms/listsort.html
        // https://www.chiark.greenend.org.uk/~sgtatham/algorithms/listsort.c
        //                      - vereena, 5/25/21

        //for(auto child : block_profiles) {
        //    child->sort();
        //}
        //qsort(block_profiles.data, block_profiles.count, sizeof(Block_Profile*), _cmp_block_profiles);
    }
};

// TODO: Fix the performance of this! It's WAY too slow right now.
struct Profiler {
    Pool<Block_Profile> block_profile_pool;

    Static_Array<Debug_Event, MAX_DEBUG_EVENTS> frame_events;
    u64 frame_events_high_water = 0;
    // Ticket_Mutex frame_events_lock;
    Frame_Profile frame_profiles[MAX_FRAME_PROFILES];
    u32 frame_profile_index = 0;
    u32 selected_frame_profile_index = 0;
    u64 frame_count = 0;

    u64 allocd;
    u64 freed;

    Profiler() : block_profile_pool(Pool<Block_Profile>(MAX_BLOCK_PROFILES)) {
        for(u32 i = 0; i < MAX_FRAME_PROFILES; i++) {
            frame_profiles[i].p = this;
        }
    }

    void deinit() {
        for(u32 i = 0; i < MAX_FRAME_PROFILES; i++) {
            frame_profiles[i].deinit();
        }
    }

    void push_event(Debug_Event e) {
        assert(pthread_self() == main_thread); // TODO
        frame_events.push(e);
        if(frame_events.count > frame_events_high_water) frame_events_high_water = frame_events.count;
    }

    void begin_frame() {
        frame_profile_index++;
        if(frame_profile_index >= MAX_FRAME_PROFILES) frame_profile_index = 0;
    }

    void end_frame() {
        Array<Block_Profile*> block_profile_stack;
        GUID current_guid;
        Array<GUID> guid_stack;
        Hash_Table<GUID, Block_Profile*, default_hash_fn, guid_eq_fn> block_profiles;
        block_profiles.init();

        Block_Profile *current_block_profile = NULL;

		Frame_Profile& fp = frame_profiles[frame_profile_index];
		fp.frame = frame_count;
		fp.clear(&Profiler::free_block_profile);

        u64 bps_allocd = 0;

        for(u32 event_index = 0; event_index < frame_events.count; event_index++) {
			auto const& event = frame_events[event_index];

			switch(event.type) {
				case DEBUG_EVENT_BLOCK_START: {
                    guid_stack.push(current_guid);

                    meow_state hash;
                    MeowBegin(&hash, MeowDefaultSeed);
                    MeowAbsorb(&hash, sizeof(GUID), &current_guid);
                    MeowAbsorb(&hash, strlen(event.guid), event.guid);
                    auto guid = GUID(MeowEnd(&hash, NULL));

					Block_Profile *this_block_profile;
					auto bpi = block_profiles.index_of(guid);
					if(bpi == -1) {
						this_block_profile = block_profile_pool.alloc();
                        pnew(Block_Profile, this_block_profile, this, guid, event.name, event.file, event.line);
                        assert(this_block_profile->prev == NULL);
                        assert(this_block_profile->children == NULL);

                        bps_allocd++;

						if(current_block_profile) {
                            this_block_profile->prev = current_block_profile->children;
                            current_block_profile->children = this_block_profile;
						} else {
                            this_block_profile->prev = fp.block_profiles;
                            fp.block_profiles = this_block_profile;
						}

						block_profiles.set(guid, this_block_profile);
					} else {
						this_block_profile = block_profiles.slots[bpi].value;
					}

					block_profile_stack.push(current_block_profile);
					current_block_profile = this_block_profile;

					current_block_profile->time -= event.time;
					break;
				}
				case DEBUG_EVENT_BLOCK_END: {
					current_block_profile->time += event.time;
					current_block_profile->count++;

					current_block_profile = block_profile_stack.pop();

                    current_guid = guid_stack.pop();
					break;
				}
				default:
					assert(0);
					break;
			}
		}

        fp.debug_events = frame_events.count;
        fp.total_block_profiles = bps_allocd;
        allocd = bps_allocd;

        assert(block_profile_stack.count == 0);
        assert(guid_stack.count == 0);
		assert(current_block_profile == NULL);

        for(auto child = fp.block_profiles; child != NULL; child = child->prev) {
            child->calculate_child_time();
        }

        fp.sort_block_profiles(); // TODO: This seems to be not working?

		block_profile_stack.free();
        guid_stack.free();
        block_profiles.free();

		selected_frame_profile_index = frame_profile_index;
	    frame_count++;

        clear_frame_events();
    }

    void clear_frame_events() {
        frame_events.clear();
    }

    void show() {
		if(ImGui::Begin("Profiler", 0, ImGuiWindowFlags_NoNav)) {
			show_frames();

			ImGui::Separator();

			Frame_Profile& fp = frame_profiles[selected_frame_profile_index]; // NOTE: - 1 because we increment after write
			ImGui::Text("Frame: %llu", fp.frame);
            ImGui::Text("Debug Events: %llu", fp.debug_events);
            ImGui::Text("Max Debug Events: %llu", frame_events_high_water);
            ImGui::Text("Block Profiles:");
            ImGui::Indent();
            ImGui::Bullet();
            ImGui::Text("Allocated: %llu", allocd);
            ImGui::Bullet();
            ImGui::Text("Freed: %llu", freed);
            ImGui::Bullet();
            ImGui::Text("Total: %llu", block_profile_pool.allocated());
            ImGui::Unindent();

            auto flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable;
            if(ImGui::BeginTable("Block Profiles", 5, flags)) {
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide);
                ImGui::TableSetupColumn("Count", ImGuiTableColumnFlags_None);
                ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_None);
                ImGui::TableSetupColumn("Self", ImGuiTableColumnFlags_None);
                ImGui::TableSetupColumn("Child", ImGuiTableColumnFlags_None);
                ImGui::TableHeadersRow();
                for(auto bp = fp.block_profiles; bp != NULL; bp = bp->prev) {
                    show_block_profile(bp);
                }
                ImGui::EndTable();
            }
		}
		ImGui::End();
        freed = 0;
	}

private:
    void free_block_profile(Block_Profile *bp) {
        // NOTE TODO: Not needed; added while trying to debug
        // a bug that only happens in release mode where
        // the profiler, as soon as I load a world, starts
        // allocating WAY more memory than it should.
        // And then the game infloops somehwere for some reason,
        // presumably in the profiler.
        // I still haven't fixed this bug but I've decided
        // I need to commit, so I'm leaving any debugging code
        // in for now. :/
        //                  - vereena, 5/25/21
        auto tmp = bp->_pooled;
        memset(bp, 0, sizeof(Block_Profile));
        bp->_pooled = tmp;
        
        block_profile_pool.free(bp);
        freed++;
    }

    void show_block_profile(Block_Profile *bp) {
		ImGui::TableNextRow();
        ImGui::TableNextColumn();

        auto flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth;
        if(bp->children != NULL) {
            flags = ImGuiTreeNodeFlags_SpanFullWidth;
        }

        char guid[sizeof(meow_u128) * 2 + 1];
        sprintf(guid, "%8X%8X", bp->guid.hi, bp->guid.lo);

        bool open = ImGui::TreeNodeEx(guid, flags, "%s", bp->name);
        ImGui::TableNextColumn();
        ImGui::Text("%u", bp->count); // TODO: comma format
        ImGui::TableNextColumn();
        ImGui::Text("%llu", bp->time); // TODO: comma format
        ImGui::TableNextColumn();
        ImGui::Text("%llu", bp->time - bp->child_time); // TODO: comma format
        ImGui::TableNextColumn();
        ImGui::Text("%llu", bp->child_time); // TODO: comma format

        if(bp->children != NULL && open) {
            for(auto child = bp->children; child != NULL; child = child->prev) {
                show_block_profile(child);
            }
            ImGui::TreePop();
        }
	}

    void show_frames() {
		// TODO: Figure out how to make this a proper
		// imgui component instead of just using ImGui::Dummy
		// Doing it the way we are now prevents scrolling from
		// working correctly (er, at all.)

		auto window_pos = ImGui::GetWindowPos();
		auto mouse_pos = ImGui::GetMousePos();
		auto dl = ImGui::GetWindowDrawList();

		constexpr f32 FRAME_WIDTH = 12.0f;
		constexpr f32 FRAME_HEIGHT = 40.0f;
		constexpr vec2 OFFSET = vec2(10.0f, 30.0f);

		vec2 min = vec2(window_pos) + OFFSET;
		vec2 max = min + vec2(FRAME_WIDTH * MAX_FRAME_PROFILES, 0) + vec2(0, FRAME_HEIGHT);

		ImGui::Dummy(max - min + vec2(10.0f, 10.0f));

		for(u32 i = 0; i < MAX_FRAME_PROFILES; i++) {
			vec2 s = min + vec2(i * FRAME_WIDTH, 0);
			vec2 e = s + vec2(FRAME_WIDTH, FRAME_HEIGHT);

			if(i == frame_profile_index) dl->AddRectFilled(s, e, 0xFF0000FF);
			else if(i == selected_frame_profile_index) dl->AddRectFilled(s, e, 0xFFFF0000);
			else if(i < frame_profile_index) dl->AddRectFilled(s, e, 0xFF00FF00);
			else if(i > frame_profile_index) dl->AddRectFilled(s, e, 0xFF00FFFF);

			if(mouse_pos.x >= s.x && mouse_pos.x < e.x && mouse_pos.y >= s.y && mouse_pos.y < e.y) {
				dl->AddRectFilled(s, e, 0x66FFFFFF);

				if(ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
					selected_frame_profile_index = i;
				}
			}

			if(i == 0) continue;

			vec2 ls = min + vec2(i * FRAME_WIDTH, 0);
			vec2 le = s + vec2(0, FRAME_HEIGHT - 1);
			dl->AddLine(ls, le, 0xFFFFFFFF);
		}

		dl->AddRect(min, max, 0xFFFFFFFF);
	}
};


Profiler profiler;


struct Timed_Block {
    char *guid;
    char *name;
    char *file;
    u32 line;

    Timed_Block(char *guid, char *name, char *file, u32 line) {
        this->guid = guid;
        this->name = name;
        this->file = file;
        this->line = line;

        Debug_Event e;
        e.type = DEBUG_EVENT_BLOCK_START;
        e.guid = guid;
        e.name = name;
        e.file = file;
        e.line = line;
        e.time = __rdtsc();
        profiler.push_event(e);
    }

    ~Timed_Block() {
        Debug_Event e;
        e.type = DEBUG_EVENT_BLOCK_END;
        e.guid = guid;
        e.name = name;
        e.file = file;
        e.line = line;
        e.time = __rdtsc();
        profiler.push_event(e);
    }
};

#define DEBUG_NAME__(a, b) a "|" #b
#define DEBUG_NAME_(a, b) DEBUG_NAME__(a, b)
#define DEBUG_NAME() DEBUG_NAME_(__FILE__, __LINE__)
#define TIMED_BLOCK_(name, file, line) Timed_Block __timed_block_##LINE__((char*)DEBUG_NAME(), (char*)name, (char*)file, line);
#define TIMED_BLOCK(name) TIMED_BLOCK_(name, __FILE__, __LINE__)
#define TIMED_FUNCTION() TIMED_BLOCK_(__PRETTY_FUNCTION__, __FILE__, __LINE__)

//#define TIMED_BLOCK(name)
//#define TIMED_FUNCTION()

#else

#define TIMED_BLOCK(name)
#define TIMED_FUNCTION()

#endif