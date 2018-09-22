#pragma once
#include "../TempEx7_SceneGraph/Scene.h"
#include "Rect.h"

class UIDemo
{
public:
    UIDemo();
    virtual ~UIDemo();

    void Grid(Scene* m_Scene, int p_Width, int p_Height);
    void MixerView(Scene* m_Scene, int p_Width, int p_Height);
    void ProgressBarFunc(Scene* m_Scene);
};

class Rectangl;
class QMouseEvent;
class ProgressBar : public Node
{
public:
    ProgressBar(Scene* p_Scene, Node* p_Parent, const BoundingRegion& p_BoundedRegion, const QString& p_Name = "", SHAPE p_ShapeType = SHAPE::SHAPE_NONE);
    virtual ~ProgressBar() {}

    virtual bool mouseMoveEvent(QMouseEvent* p_Event);
    Rectangl* progressIndicator;
    Rectangl* bar;
};

class AudioMixerItem : public Node
{
public:
    AudioMixerItem(Scene* p_Scene, Node* p_Parent, const BoundingRegion& p_BoundedRegion, const QString& p_Name, SHAPE p_ShapeType = SHAPE::SHAPE_NONE);
    virtual ~AudioMixerItem() {}

    //virtual bool mouseMoveEvent(QMouseEvent* p_Event);
    //Node* progressIndicator;
};
