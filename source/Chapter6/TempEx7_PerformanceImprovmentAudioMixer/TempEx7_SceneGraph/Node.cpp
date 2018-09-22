#include "Node.h"
#include<QMouseEvent>
#include<glm/gtx/string_cast.hpp>

Node::Node(Scene *p_Scene, Node *p_Parent, const BoundingRegion& p_BoundedRegion, const QString &p_Name, SHAPE p_ShapeType, RENDER_SCEHEME_TYPE p_RenderSchemeType)
    : m_Scene(p_Scene)
    , m_Parent(p_Parent)
    , m_ShapeType(p_ShapeType)
    , m_BoundedRegion(p_BoundedRegion)
    , m_RenderSchemeType(p_RenderSchemeType)
    , m_Visible(true)
    , m_DirtyType(DIRTY_TYPE::ALL)
{
    m_Parent ? m_Parent->m_ChildList.append(this) : p_Scene->AddModel(this);

    // Todo: We can directly use the translate as the m_BoundedRegion is already set
    SetGeometry(m_BoundedRegion.m_Position.x, m_BoundedRegion.m_Position.y, m_BoundedRegion.m_Dimension.x, m_BoundedRegion.m_Dimension.y, m_BoundedRegion.m_Position.z);
}

Node::~Node()
{
    if (m_Scene)
    {
        m_Scene->RemoveModel(this);
    }
}

void Node::Setup()
{
    foreach(Node* currentModel, m_ChildList)
    {
        Node* model = /*dynamic_cast<Node*>*/(currentModel);

        if (!model) continue;

        model->Setup();
    }
}

void Node::mousePressEvent(QMouseEvent* p_Event)
{
    glm::vec4 posStart((0 * m_BoundedRegion.m_Dimension.x), (0 * m_BoundedRegion.m_Dimension.y), 0.0, 1.0);
    glm::vec4 posStartResult = m_ModelTransformation * posStart;

    glm::vec4 posEnd((m_BoundedRegion.m_Dimension.x), (m_BoundedRegion.m_Dimension.y), 0.0, 1.0);
    glm::vec4 posEndResult = m_ModelTransformation * posEnd;

    cout << "\n##### mousePressEventS" << glm::to_string(posStartResult);// << posEndResult;
    cout << "\n##### mousePressEventE" << glm::to_string(posEndResult);// << posEndResult;

    QRect rect(QPoint(posStartResult.x, posStartResult.y), QPoint(posEndResult.x, posEndResult.y));
    if (rect.contains(p_Event->x(), p_Event->y()))
        cout << "\n***************";


    foreach(Node* item, m_ChildList)
    {
        assert(item);

        item->mousePressEvent(p_Event);
    }
}

void Node::mouseReleaseEvent(QMouseEvent* p_Event)
{
//    return;

	//cout << "\n##### mouseReleaseEvent";
    foreach(Node* item, m_ChildList)
	{
		assert(item);

		item->mouseReleaseEvent(p_Event);
	}
}

bool Node::mouseMoveEvent(QMouseEvent* p_Event)
{
    glm::vec4 posStart((0 * m_BoundedRegion.m_Dimension.x), (0 * m_BoundedRegion.m_Dimension.y), 0.0, 1.0);
    glm::vec4 posStartResult = /*GetParentsTransformation(GetParent()) **/ m_ModelTransformation * posStart;

    glm::vec4 posEnd((m_BoundedRegion.m_Dimension.x), (m_BoundedRegion.m_Dimension.y), 0.0, 1.0);
    glm::vec4 posEndResult = /*GetParentsTransformation(GetParent()) **/ m_ModelTransformation * posEnd;

    QRect rect(QPoint(posStartResult.x, posStartResult.y), QPoint(posEndResult.x, posEndResult.y));
    if (rect.contains(p_Event->x(), p_Event->y()))
    {
        m_Scene->SetCurrentHoverItem(this);

        //cout << "\n##### mouseMoveEvent";
        for (int i = m_ChildList.size() - 1; i >= 0; i--)
        {
            Node* item = m_ChildList.at(i);
            assert(item);

            if (item->mouseMoveEvent(p_Event))
            {
                return true;
            }
        }

        return true;
    }

    return false;
}

void Node::Update()
{
    m_Scene->PushMatrix();
    m_Scene->ApplyTransformation(m_Model);

    m_ModelTransformation = *m_Scene->m_Transform.GetModelMatrix();

    Q_FOREACH(Node* child, m_ChildList)
    {
        assert(child);
        child->Update();
    }

    m_Scene->PopMatrix();
}

//void Node::Rectangle(float p_X, float p_Y, float p_Width, float p_Height, float p_ZOrder/*=0*/)
//{
//    Translate(p_X, p_Y, p_ZOrder);

//    m_BoundedRegion.m_Position.x = p_X;
//    m_BoundedRegion.m_Position.y = p_Y;
//    m_BoundedRegion.m_Position.z = p_ZOrder;

//    m_BoundedRegion.m_Dimension.x = p_Width;
//    m_BoundedRegion.m_Dimension.y = p_Height;
//}

void Node::SetZOrder(float p_ZOrder)
{
    m_BoundedRegion.m_Position.z = p_ZOrder;

    Reset();
    Translate(m_BoundedRegion.m_Position.x, m_BoundedRegion.m_Position.y, m_BoundedRegion.m_Position.z);
}

void Node::SetPosition(float p_X, float p_Y)
{
    glm::vec4 posStart((0 * m_BoundedRegion.m_Dimension.x), (0 * m_BoundedRegion.m_Dimension.y), 0.0, 1.0);
    glm::vec4 posStartResult = /*GetParentsTransformation(GetParent()) **/ m_ModelTransformation * posStart;

    glm::vec4 posEnd((m_BoundedRegion.m_Dimension.x), (m_BoundedRegion.m_Dimension.y), 0.0, 1.0);
    glm::vec4 posEndResult = /*GetParentsTransformation(GetParent()) **/ m_ModelTransformation * posEnd;

    m_BoundedRegion.m_Position.x = p_X;
    m_BoundedRegion.m_Position.y = p_Y;

    Reset();
    Translate(m_BoundedRegion.m_Position.x, m_BoundedRegion.m_Position.y, m_BoundedRegion.m_Position.z);
    m_ModelTransformation = m_Model * GetParentsTransformation(GetParent());
}

void Node::SetGeometry(float p_X, float p_Y, float p_Width, float p_Height, float p_ZOrder/*=0*/)
{
    Translate(p_X, p_Y, p_ZOrder);

    m_BoundedRegion.m_Position.x = p_X;
    m_BoundedRegion.m_Position.y = p_Y;
    m_BoundedRegion.m_Position.z = p_ZOrder;

    m_BoundedRegion.m_Dimension.x = p_Width;
    m_BoundedRegion.m_Dimension.y = p_Height;
}

// When a Model is updated it may need recomputation of the transformation
void Node::SetDirtyType(DIRTY_TYPE p_InvalidateType)
{
    m_DirtyType = p_InvalidateType;

    if (m_Scene && IsDirty())
    {
        const DIRTY_TYPE isPositionUpdated = static_cast<DIRTY_TYPE>(static_cast<int>(m_DirtyType) & static_cast<int>(DIRTY_TYPE::POSITION));
        if (isPositionUpdated == DIRTY_TYPE::POSITION)
        {
            m_Scene->SetDirtyType(static_cast<SCENE_DIRTY_TYPE>(static_cast<int>(m_Scene->GetDirtyType()) | static_cast<int>(SCENE_DIRTY_TYPE::TRANSFORMATION)));
        }

        const DIRTY_TYPE isAttributeUpdated = static_cast<DIRTY_TYPE>(static_cast<int>(m_DirtyType) & static_cast<int>(DIRTY_TYPE::ATTRIBUTES));
        if (isAttributeUpdated == DIRTY_TYPE::ATTRIBUTES)
        {
            m_Scene->SetDirtyType(static_cast<SCENE_DIRTY_TYPE>(static_cast<int>(m_Scene->GetDirtyType()) | static_cast<int>(SCENE_DIRTY_TYPE::DIRTY_ITEMS)));
        }
    }
}

void Node::GatherFlatList()
{
    m_Scene->m_FlatList.push_back(this);

    Q_FOREACH(Node* child, m_ChildList)
    {
        assert(child);
        child->GatherFlatList();
    }
}
