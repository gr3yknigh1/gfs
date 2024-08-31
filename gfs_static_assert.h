#if !defined(GFS_STATIC_ASSERT_H_INCLUDED)
/*
 * FILE      gfs_static_assert.h
 * AUTHOR    Ilya Akkuzin <gr3yknigh1@gmail.com>
 * COPYRIGHT (c) 2024 Ilya Akkuzin
 * */
#define GFS_STATIC_ASSERT_H_INCLUDED

#define STATIC_ASSERT(COND) _Static_assert((COND), "")
#define EXPECT_TYPE_SIZE(TYPE, SIZE) STATIC_ASSERT(sizeof(TYPE) == (SIZE))

#endif // GFS_STATIC_ASSERT_H_INCLUDED
