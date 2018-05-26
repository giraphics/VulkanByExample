#include "Model3D.h"
#include<QMouseEvent>
#include<glm/gtx/string_cast.hpp>

Model3D::Model3D(Scene3D *p_Scene, Model3D *p_Parent, const QString &p_Name, SHAPE p_ShapeType, RENDER_SCEHEME_TYPE p_RenderSchemeType)
    : m_Scene(p_Scene)
    , m_Parent(p_Parent)
    , m_ShapeType(p_ShapeType)
	, m_RenderSchemeType(p_RenderSchemeType)
{
    m_Parent ? m_Parent->m_ChildList.append(this) : p_Scene->AddModel(this);
}

void Model3D::Setup()
{
    foreach(Model3D* currentModel, m_ChildList)
    {
        Model3D* model = /*dynamic_cast<Model3D*>*/(currentModel);

        if (!model) continue;

        model->Setup();
    }
}

void Model3D::mousePressEvent(QMouseEvent* p_Event)
{
	glm::vec4 posStart((0 * m_Dimension.x), (0 * m_Dimension.y), 0.0, 1.0);
	glm::vec4 posStartResult = m_TransformedModel * posStart;

	glm::vec4 posEnd((m_Dimension.x), (m_Dimension.y), 0.0, 1.0);
	glm::vec4 posEndResult = m_TransformedModel * posEnd;

	cout << "\n##### mousePressEventS" << glm::to_string(posStartResult);// << posEndResult;
	cout << "\n##### mousePressEventE" << glm::to_string(posEndResult);// << posEndResult;

	QRect rect(QPoint(posStartResult.x, posStartResult.y), QPoint(posEndResult.x, posEndResult.y));
	if (rect.contains(p_Event->x(), p_Event->y()))
		cout << "\n***************";
	

	foreach(Model3D* item, m_ChildList)
	{
		assert(item);

		item->mousePressEvent(p_Event);
	}
}

void Model3D::mouseReleaseEvent(QMouseEvent* p_Event)
{
	//cout << "\n##### mouseReleaseEvent";
	foreach(Model3D* item, m_ChildList)
	{
		assert(item);

		item->mouseReleaseEvent(p_Event);
	}
}

bool Model3D::mouseMoveEvent(QMouseEvent* p_Event)
{
	glm::vec4 posStart((0 * m_Dimension.x), (0 * m_Dimension.y), 0.0, 1.0);
	glm::vec4 posStartResult = /*GetParentsTransformation(GetParent()) **/ m_TransformedModel * posStart;

	glm::vec4 posEnd((m_Dimension.x), (m_Dimension.y), 0.0, 1.0);
	glm::vec4 posEndResult = /*GetParentsTransformation(GetParent()) **/ m_TransformedModel * posEnd;

	QRect rect(QPoint(posStartResult.x, posStartResult.y), QPoint(posEndResult.x, posEndResult.y));
	if (rect.contains(p_Event->x(), p_Event->y()))
	{
		m_Scene->SetCurrentHoverItem(this);

		//cout << "\n##### mouseMoveEvent";
		for (int i = m_ChildList.size() - 1; i >= 0; i--)
		{
			Model3D* item = m_ChildList.at(i);
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

void Model3D::Update()
{
    m_Scene->PushMatrix();
    m_Scene->ApplyTransformation(m_Model);

    m_TransformedModel = *m_Scene->m_Transform.GetModelMatrix();

    Q_FOREACH(Model3D* child, m_ChildList)
    {
        assert(child);
        child->Update();
    }

    m_Scene->PopMatrix();
}

void Model3D::Rectangle(float p_X, float p_Y, float p_Width, float p_Height)
{
	Translate(p_X, p_Y, 0);
	m_Dimension.x = p_Width;
	m_Dimension.y = p_Height;
	//	Scale(p_Width, p_Height, 1);
}

void Model3D::GatherFlatList()
{
    m_Scene->m_FlatList.push_back(this);

    Q_FOREACH(Model3D* child, m_ChildList)
    {
        assert(child);
        child->GatherFlatList();
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////
#include "../Chapter5/Ch5_03_UI_Shapes/source/Cube.h"
//#include "../Chapter6/Ch6_02_DrawingMultipleObjects/source/Rect.h"

ProgressBar::ProgressBar(Scene3D* p_Scene, Model3D* p_Parent, const QString& p_Name, SHAPE p_ShapeType)
	: Model3D(p_Scene, p_Parent, p_Name, p_ShapeType)
{
    Model3D* background = new RectangleModel(NULL, m_Scene, this, "Rectangle 1", SHAPE_RECTANGLE);
	background->Rectangle(0, 0, 400, 50);
	background->SetColor(glm::vec4(0.6, 01.0, 0.0, 1.0));
	background->SetDefaultColor(glm::vec4(0.42, 0.65, 0.60, 1.0));

    Model3D* bar = new RectangleModel(NULL, m_Scene, background, "Bar", SHAPE_RECTANGLE);
    bar->Rectangle(0, (background->GetDimension().y * 0.25f), 400, 25);
	bar->SetColor(glm::vec4(0.6, 0.52, 0.320, 1.0));
	bar->SetDefaultColor(glm::vec4(0.42, 0.15, 0.60, 1.0));
    
    progressIndicator = new RectangleModel(NULL, m_Scene, bar, "ProgressIndicator", SHAPE_RECTANGLE);
    progressIndicator->Rectangle(0, 0, 20, background->GetDimension().y);
	progressIndicator->Translate(0, -(background->GetDimension().y * 0.25f), 0);
	progressIndicator->SetColor(glm::vec4(0.1, 0.52, 0.320, 1.0));
	progressIndicator->SetDefaultColor(glm::vec4(0.2, 0.15, 0.60, 1.0));
}

bool ProgressBar::mouseMoveEvent(QMouseEvent* p_Event)
{
    if (progressIndicator->mouseMoveEvent(p_Event))
    {
        progressIndicator->Translate(p_Event->x(), 0.0, 0.0);
        return true;
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////

AudioMixerItem::AudioMixerItem(Scene3D* p_Scene, Model3D* p_Parent, const QString& p_Name, SHAPE p_ShapeType)
    : Model3D(p_Scene, p_Parent, p_Name, p_ShapeType)
{
    //RectangleFactory* m_CubeFactory = RectangleFactory::SingleTon(p_VulkanApp);
    //m_AbstractFactory = m_CubeFactory; // this is not correct, cube factory should be a part of
//    background->Rectangle(300, 300, 400, 50);
//    background->SetColor(glm::vec4(0.6, 0.52, 1.0, 1.0));
//    background->SetDefaultColor(glm::vec4(0.42, 0.65, 0.60, 1.0));

//    Model3D* bar = m_CubeFactory->GetModel(p_VulkanApp, m_Scene, background, "Bar", SHAPE_CUBE);
//    bar->Rectangle(0, (background->GetDimension().y * 0.25f), 400, 25);
//    bar->SetColor(glm::vec4(0.6, 0.52, 0.320, 1.0));
//    bar->SetDefaultColor(glm::vec4(0.42, 0.15, 0.60, 1.0));

//    progressIndicator = m_CubeFactory->GetModel(p_VulkanApp, m_Scene, bar, "ProgressIndicator", SHAPE_CUBE);
//    progressIndicator->Rectangle(0, 0, 20, background->GetDimension().y);
//    progressIndicator->Translate(0, -(background->GetDimension().y * 0.25f), 0);
//    progressIndicator->SetColor(glm::vec4(0.1, 0.52, 0.320, 1.0));
//    progressIndicator->SetDefaultColor(glm::vec4(0.2, 0.15, 0.60, 1.0));
}

//bool AudioMixerItem::mouseMoveEvent(QMouseEvent* p_Event)
//{
//    return false;
//}
