#ifndef SCENE_H
#define SCENE_H

#include <QObject>
#include <QMatrix4x4>

#include "Transformation3D.h"
#include "../VulkanHelper.h"
//#include "Model3D.h"
class Model3D;
class UiFL3DRenderableItem;
class QWindow;
class Scene3D;

class Scene3D
{
public:
    Scene3D(const QString& p_Name = "");
    virtual ~Scene3D();

    void Setup();
	void Update();
    void UpdateNew();

    void AddModel(Model3D* p_Model);
    void RemoveModel(Model3D *p_Model);

    void Resize(int p_Width, int p_Height);
    void SetUpProjection();
    inline Transformation3D& Transform() { return m_Transform; }
    //GETSET(glm::mat4*, Projection)	// Not owned by Scene, double check this can be owned. TODO: PS
    //GETSET(glm::mat4*, View)		// Not owned by Scene
	
	void PushMatrix() { m_Transform.PushMatrix(); }
	void PopMatrix() { m_Transform.PopMatrix(); }
	void ApplyTransformation(const glm::mat4& m_TransformationMatrix)
	{
		*m_Transform.GetModelMatrix() *= m_TransformationMatrix;
	}

public:
    int m_ScreenHeight;
    int m_ScreenWidth;

	QList<Model3D*> m_ModelList;
    Transformation3D m_Transform;
    int m_Frame;
public:
	std::vector<QMatrix4x4> m_MatrixVector;
	std::vector<Model3D*> m_FlatList;
};
#endif // SCENE_H
