#pragma once

#define M_PI (3.14)
#define M_PI_2 (3.14 * 2)

enum class SHAPE : int32_t
{
    SHAPE_NONE = -1,

    // BASIC SHAPES GOES HERE
    SHAPE_CUBE,
    SHAPE_RECTANGLE,

    SHAPE_COUNT, // TOTAL SHAPE COUNTS

    // CUSTOM SHAPES GOES HERE
    SHAPE_CUSTOM // NOTE: Important: the custom object are comprised of simpler objects hence not a part of model factories
};

enum class RENDER_SCEHEME_TYPE : int32_t
{
    RENDER_SCEHEME_NONE         = -1,
    RENDER_SCEHEME_INSTANCED,               // Single Command buffer, Single Draw call
    RENDER_SCEHEME_MULTIDRAW,               // Single Command buffer, Multiple Draw call
    RENDER_SCEHEME_COUNT
};

enum class DIRTY_TYPE : uint32_t
{
    NONE       = (0u),
    ATTRIBUTES = (1u << 0u),
    POSITION   = (1u << 1u),
    ALL        = (POSITION | ATTRIBUTES),
};

enum class SCENE_DIRTY_TYPE : uint32_t
{
    NONE            = (0u),
    TRANSFORMATION  = (1u << 0u),
    ALL_ITEMS       = (1u << 1u),
    DIRTY_ITEMS     = (1u << 2u),
    ALL             = (TRANSFORMATION | ALL_ITEMS),
};
