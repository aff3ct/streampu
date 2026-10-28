#ifndef HELGRIND_ANNOTATIONS_H_
#define HELGRIND_ANNOTATIONS_H_

#ifdef SPU_HAS_HELGRIND
#include <valgrind/helgrind.h>
#else
#define ANNOTATE_HAPPENS_BEFORE(x) ((void)0)
#define ANNOTATE_HAPPENS_AFTER(x) ((void)0)
#define ANNOTATE_HAPPENS_BEFORE_FORGET_ALL() ((void)0)
#endif

#endif // HELGRIND_ANNOTATIONS_H_