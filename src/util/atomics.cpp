// NOTE: I debated between __atomic* and __sync* and ended up deciding to stick with 
// __sync* for now, mainly because in my (probably insufficient) testing with Godbolt,
// I could find no difference whatsoever between them. Though I wouldn't be surprised
// if that's coincidental. *shrugs*
// I don't think we _need_ to specify the memory ordering for anything right now,
// and I doubt we could "optimize" much/at all by doing so, so for now, sticking with
// the old and, I guess, deprecated functions. Revisit if called for.


// NOTE: These functions return the _original_ value prior to applying the atomic operator.

#define DEFATOMICS(t, _)                                                    \
    t atomic_add(t volatile* p, t v) { return __sync_fetch_and_add(p, v); } \
    t atomic_sub(t volatile* p, t v) { return __sync_fetch_and_sub(p, v); } \
    t atomic_or(t volatile* p, t v) { return __sync_fetch_and_or(p, v); }   \
    t atomic_and(t volatile* p, t v) { return __sync_fetch_and_and(p, v); } \
    t atomic_xor(t volatile* p, t v) { return __sync_fetch_and_xor(p, v); }
INTEGRAL_TYPES(DEFATOMICS)
#undef DEFATOMICS

#define DEFCAS(t, _) t compare_and_swap(t volatile* p, t ov, t nv) { return __sync_val_compare_and_swap(p, ov, nv); }
INTEGRAL_TYPES(DEFCAS)
#undef DEFCAS