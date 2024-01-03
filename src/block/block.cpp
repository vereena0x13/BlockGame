bool Block::should_draw_face(World *world, vec3i pos, Direction dir) {
    auto block = world->get_block(pos + DIRECTION_OFFSET[dir]);
    if(!block || block == block_air) return true;
    return !block->is_solid() || (!is_transparent() && block->is_transparent());
}


#include "simple.cpp"
#include "air.cpp"
#include "grass.cpp"
#include "glass.cpp"
#include "furnace.cpp"


Block* BLOCKS[1024];
blkid N_BLOCKS = 0;


#define X(name) Block *block_##name;
BLOCKS(X)
#undef X


void register_blocks() {
    auto& id = N_BLOCKS;

    #define add_block(vn, block) BLOCKS[id] = block_##vn = block; id++;

    add_block(air, xnew(Block_Air, id));
    add_block(bedrock, xnew(Block_Simple, id, "blocks/bedrock"));
    add_block(stone, xnew(Block_Simple, id, "blocks/stone"));
    add_block(dirt, xnew(Block_Simple, id, "blocks/dirt"));
    add_block(grass, xnew(Block_Grass, id));
    add_block(sand, xnew(Block_Simple, id, "blocks/sand"));
    add_block(glass, xnew(Block_Glass, id));
    add_block(iron_ore, xnew(Block_Simple, id, "blocks/iron_ore"));
    add_block(iron, xnew(Block_Simple, id, "blocks/iron_block"));
    add_block(gold_ore, xnew(Block_Simple, id, "blocks/gold_ore"));
    add_block(gold, xnew(Block_Simple, id, "blocks/gold_block"));
    add_block(furnace, xnew(Block_Furnace, id));

    #undef add_block

    log(DEBUG, "Blocks initialized");
}