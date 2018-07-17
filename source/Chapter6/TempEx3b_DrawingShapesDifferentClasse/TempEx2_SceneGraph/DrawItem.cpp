#include "DrawItem.h"
#include<QMouseEvent>
#include<glm/gtx/string_cast.hpp>

DrawItem::DrawItem(Scene* p_Scene, DrawItem* p_Parent, const BoundingRegion& p_BoundedRegion, const QString& p_Name, SHAPE p_ShapeType)
    : m_Scene(p_Scene)
    , m_Parent(p_Parent)
    , m_Name(p_Name)
    , m_ShapeType(p_ShapeType)
    , m_BoundedRegion(p_BoundedRegion)
    , m_OriginOffset(glm::vec3(0.0f, 0.0f, 0.0f))
{
    m_Parent ? m_Parent->m_ChildList.append(this) : p_Scene->AddItem(this);
    
    // Todo: We can directly use the translate as the m_BoundedRegion is already set
    SetGeometry(m_BoundedRegion.m_Position.x, m_BoundedRegion.m_Position.y, m_BoundedRegion.m_Dimension.x, m_BoundedRegion.m_Dimension.y, m_BoundedRegion.m_Position.z);
}

void DrawItem::Setup()
{
    foreach(DrawItem* childItem, m_ChildList)
    {
        if (!childItem) continue;

        childItem->Setup();
    }
}

void DrawItem::mousePressEvent(QMouseEvent* p_Event)
{
    glm::vec4 posStart((0 * m_BoundedRegion.m_Dimension.x), (0 * m_BoundedRegion.m_Dimension.y), 0.0, 1.0);
    glm::vec4 posStartResult = m_TransformedModel * posStart;

    glm::vec4 posEnd((m_BoundedRegion.m_Dimension.x), (m_BoundedRegion.m_Dimension.y), 0.0, 1.0);
    glm::vec4 posEndResult = m_TransformedModel * posEnd;

    cout << "\n##### mousePressEventS" << glm::to_string(posStartResult);// << posEndResult;
    cout << "\n##### mousePressEventE" << glm::to_string(posEndResult);// << posEndResult;

    QRect rect(QPoint(posStartResult.x, posStartResult.y), QPoint(posEndResult.x, posEndResult.y));
    if (rect.contains(p_Event->x(), p_Event->y()))
        cout << "\n***************";

    foreach(DrawItem* childItem, m_ChildList)
    {
        assert(childItem);

        childItem->mousePressEvent(p_Event);
    }
}

void DrawItem::mouseReleaseEvent(QMouseEvent* p_Event)
{
    foreach(DrawItem* childItem, m_ChildList)
    {
        assert(childItem);

        childItem->mouseReleaseEvent(p_Event);
    }
}

bool DrawItem::mouseMoveEvent(QMouseEvent* p_Event)
{
    glm::vec4 posStart((0 * m_BoundedRegion.m_Dimension.x), (0 * m_BoundedRegion.m_Dimension.y), 0.0, 1.0);
    glm::vec4 posStartResult = /*GetParentsTransformation(GetParent()) **/ m_TransformedModel * posStart;

    glm::vec4 posEnd((m_BoundedRegion.m_Dimension.x), (m_BoundedRegion.m_Dimension.y), 0.0, 1.0);
    glm::vec4 posEndResult = /*GetParentsTransformation(GetParent()) **/ m_TransformedModel * posEnd;

    QRect rect(QPoint(posStartResult.x, posStartResult.y), QPoint(posEndResult.x, posEndResult.y));
    if (rect.contains(p_Event->x(), p_Event->y()))
    {
        m_Scene->SetCurrentHoverItem(this);

        //cout << "\n##### mouseMoveEvent";
        for (int i = m_ChildList.size() - 1; i >= 0; i--)
        {
            DrawItem* childItem = m_ChildList.at(i);
            assert(childItem);

            if (childItem->mouseMoveEvent(p_Event))
            {
                return true;
            }
        }

        return true;
    }

    return false;
}

DrawItem* DrawItem::GetParent() const
{
    return m_Parent;
}

void DrawItem::ApplyTransformation()
{
    *m_Scene->Transform().GetModelMatrix() *= m_Model;
}

glm::mat4 DrawItem::GetRelativeTransformations() const
{
    return GetParentsTransformation(GetParent()) * m_Model;
}

glm::mat4 DrawItem::GetParentsTransformation(DrawItem *p_Parent) const
{
    return p_Parent ? (GetParentsTransformation(p_Parent->GetParent()) * p_Parent->m_Model) : glm::mat4();
}

void DrawItem::Update(DrawItem* p_Item)
{
    m_Scene->PushMatrix();
    if (p_Item)
    {
        m_Scene->ApplyTransformation(p_Item->GetParentsTransformation(GetParent()) * m_Model); // This retrives all the transformation from the parent
    }
    else
    {
        m_Scene->ApplyTransformation(m_Model);
    }

    m_TransformedModel = *m_Scene->GetRefTransform().GetModelMatrix();
    
    QList<DrawItem*>& childList = (p_Item ? p_Item->m_ChildList : m_ChildList);
    Q_FOREACH(DrawItem* childItem, childList)
    {
        if (!childItem) continue;

        childItem->Update();
    }

    m_Scene->PopMatrix();
}

void DrawItem::Rotate(float p_Angle, float p_X, float p_Y, float p_Z)
{
    if (m_OriginOffset != glm::vec3(0.0f, 0.0f, 0.0f))
    {
        m_Model = glm::translate(m_Model, m_OriginOffset);
    }

    m_Model = glm::rotate(m_Model, p_Angle, glm::vec3(p_X, p_Y, p_Z));

    if (m_OriginOffset != glm::vec3(0.0f, 0.0f, 0.0f))
    {
        m_Model = glm::translate(m_Model, -m_OriginOffset);
    }
}

void DrawItem::Translate(float p_X, float p_Y, float p_Z)
{
    if (m_OriginOffset != glm::vec3(0.0f, 0.0f, 0.0f))
    {
        m_Model = glm::translate(m_Model, m_OriginOffset);
    }

    m_Model = glm::translate(m_Model, glm::vec3(p_X, p_Y, p_Z));

    if (m_OriginOffset != glm::vec3(0.0f, 0.0f, 0.0f))
    {
        m_Model = glm::translate(m_Model, -m_OriginOffset);
    }
}

void DrawItem::Scale(float p_X, float p_Y, float p_Z)
{
    if (m_OriginOffset != glm::vec3(0.0f, 0.0f, 0.0f))
    {
        m_Model = glm::translate(m_Model, m_OriginOffset);
    }

    m_Model = glm::scale(m_Model, glm::vec3(p_X, p_Y, p_Z));

    if (m_OriginOffset != glm::vec3(0.0f, 0.0f, 0.0f))
    {
        m_Model = glm::translate(m_Model, -m_OriginOffset);
    }
}

void DrawItem::SetGeometry(float p_X, float p_Y, float p_Width, float p_Height, float p_ZOrder/*=0*/)
{
    Translate(p_X, p_Y, p_ZOrder);

    m_BoundedRegion.m_Position.x = p_X;
    m_BoundedRegion.m_Position.y = p_Y;
    m_BoundedRegion.m_Position.z = p_ZOrder;

    m_BoundedRegion.m_Dimension.x = p_Width;
    m_BoundedRegion.m_Dimension.y = p_Height;
}

void DrawItem::SetZOrder(float p_ZOrder)
{
    m_BoundedRegion.m_Position.z = p_ZOrder;

    Reset();
    Translate(m_BoundedRegion.m_Position.x, m_BoundedRegion.m_Position.y, m_BoundedRegion.m_Position.z);
}

void DrawItem::SetPosition(float p_X, float p_Y)
{
    glm::vec4 posStart((0 * m_BoundedRegion.m_Dimension.x), (0 * m_BoundedRegion.m_Dimension.y), 0.0, 1.0);
    glm::vec4 posStartResult = m_TransformedModel * posStart;

    glm::vec4 posEnd((m_BoundedRegion.m_Dimension.x), (m_BoundedRegion.m_Dimension.y), 0.0, 1.0);
    glm::vec4 posEndResult = m_TransformedModel * posEnd;

    m_BoundedRegion.m_Position.x = p_X;
    m_BoundedRegion.m_Position.y = p_Y;

    Reset();
    Translate(m_BoundedRegion.m_Position.x, m_BoundedRegion.m_Position.y, m_BoundedRegion.m_Position.z);
    m_TransformedModel = m_Model * GetParentsTransformation(GetParent());
}

void DrawItem::GatherDrawItemsFlatList()
{
    if (!m_Scene) return;

    m_Scene->AppendToDrawItemsFlatList(this);

    Q_FOREACH(DrawItem* childItem, m_ChildList)
    {
        assert(childItem);
        childItem->GatherDrawItemsFlatList();
    }
}
