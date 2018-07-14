#pragma once

enum SHAPE
{
    SHAPE_NONE = -1,

    // BASIC SHAPES GOES HERE
    SHAPE_RECTANGLE_MULTIDRAW,
    SHAPE_RECTANGLE_INSTANCED,

    SHAPE_CIRCLE_MULTIDRAW,
    SHAPE_CIRCLE_INSTANCED,

    SHAPE_COUNT, // TOTAL SHAPE COUNTS

    // CUSTOM SHAPES GOES HERE
    SHAPE_CUSTOM // NOTE: Important: the custom object are comprised of simpler objects hence not a part of model factories
};

//enum RENDER_SCEHEME_TYPE
//{
//    RENDER_SCEHEME_NONE = -1,
//    RENDER_SCEHEME_INSTANCED = 1000, // Single Command buffer, Single Draw call
//    RENDER_SCEHEME_MULTIDRAW = 2000, // Single Command buffer, Multiple Draw call
//    RENDER_SCEHEME_COUNT
//};
