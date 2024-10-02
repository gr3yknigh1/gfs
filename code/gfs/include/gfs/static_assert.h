#if !defined(GFS_STATIC_ASSERT_H_INCLUDED)
/*
 * FILE      gfs_static_assert.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#define GFS_STATIC_ASSERT_H_INCLUDED

#if !defined(__cplusplus)
#define STATIC_ASSERT(COND) _Static_assert((COND), "")
#else
#define STATIC_ASSERT(COND) static_assert((COND), "")
#endif

#define EXPECT_TYPE_SIZE(TYPE, SIZE) STATIC_ASSERT(sizeof(TYPE) == (SIZE))

#endif // GFS_STATIC_ASSERT_H_INCLUDED
