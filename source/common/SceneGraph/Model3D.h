#ifndef UIFL_RENDERABLE_ITEM_H
#define UIFL_RENDERABLE_ITEM_H

#include <QPointer>
#include <assert.h>

#define M_PI (3.14)
#define M_PI_2 (3.14 * 2)

#include "Scene3D.h"
#include "../common/VulkanHelper.h"
/*********** GLM HEADER FILES ***********/
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

enum SHAPE
{
	SHAPE_NONE = -1,
	SHAPE_CUBE,
	SHAPE_COUNT
};

class Model3D
{
public:
    Model3D(Scene3D* p_Scene, Model3D* p_Parent, const QString& p_Name = "", SHAPE p_ShapeType = SHAPE::SHAPE_NONE)
		: m_Scene(p_Scene)
		, m_Parent(p_Parent)
		, m_ShapeType(p_ShapeType)
	{
		m_Parent ? m_Parent->m_ChildList.append(this) : p_Scene->AddModel(this);
	}

	// Life Cycle
	virtual void Setup()
	{
        foreach(Model3D* currentModel, m_ChildList)
		{
            Model3D* model = dynamic_cast<Model3D*>(currentModel);

			if (!model) continue;

			model->Setup();
		}
	}

    void UpdateNew();

    virtual void Update();

	void Rotate(float p_Angle, float p_X, float p_Y, float p_Z) { m_Model = glm::rotate(m_Model, p_Angle, glm::vec3(p_X, p_Y, p_Z)); }
	void Translate(float p_X, float p_Y, float p_Z) { m_Model = glm::translate(m_Model, glm::vec3(p_X, p_Y, p_Z)); }
    void Scale(float p_X, float p_Y, float p_Z) { m_Model = glm::translate(m_Model, glm::vec3(p_X, p_Y, p_Z)); }
	void Reset() { m_TransformedModel = glm::mat4(); }

	GETSET(glm::mat4, Model)		// Owned by drawable item
	GETSET(Scene3D*, Scene)
	GETSET(Model3D*, Parent)
	GETSET(glm::mat4, TransformedModel)		// Owned by drawable item

	// Mouse interaction: Dummy interface for now.
	virtual void mousePressEvent() UNIMPLEMENTED_INTEFACE
	virtual void mouseReleaseEvent() UNIMPLEMENTED_INTEFACE
	virtual void mouseMoveEvent() UNIMPLEMENTED_INTEFACE
	virtual void mouseDoubleClickEvent() UNIMPLEMENTED_INTEFACE

	// Key interaction: Dummy interface for now.
	virtual void keyPressEvent() UNIMPLEMENTED_INTEFACE

	// Application Window resizing
	virtual void ResizeWindow(int width, int height) {}

	inline Model3D* GetParent() const { return m_Parent; }
	void ApplyTransformation() { *m_Scene->Transform().GetModelMatrix() *= m_Model; }
	glm::mat4 GetRelativeTransformations() const { return GetParentsTransformation(GetParent()) * m_Model; }
	glm::mat4 GetParentsTransformation(Model3D* p_Parent) const { return p_Parent ? (GetParentsTransformation(p_Parent->GetParent()) * p_Parent->m_Model) : glm::mat4(); }

private:
    QList<Model3D*> m_ChildList;
	SHAPE m_ShapeType;
};

#endif // ABSTRACTMODEL_H
