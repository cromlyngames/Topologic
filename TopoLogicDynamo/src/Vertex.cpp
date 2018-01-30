#include <Vertex.h>
#include <Edge.h>

#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRep_Tool.hxx>
#include <Geom_CartesianPoint.hxx>
#include <TopoDS.hxx>

namespace TopoLogic
{
	Dictionary<String^, Object^>^ Vertex::ByPoint(Autodesk::DesignScript::Geometry::Point ^ point)
	{
		Dictionary<String^, Object^>^ pDictionary = gcnew Dictionary<String^, Object^>();
		try {
			Vertex^ pVertex = gcnew Vertex(point);
			pDictionary->Add("TopoLogic Vertex", pVertex);
			pDictionary->Add("Point", pVertex->Geometry);
		}
		catch (ArgumentNullException^)
		{
			throw gcnew ArgumentNullException("point");
		}
		return pDictionary;
	}

	Dictionary<String^, Object^>^ Vertex::Edges(Vertex^ topoLogicVertex)
	{
		Dictionary<String^, Object^>^ pDictionary = gcnew Dictionary<String^, Object^>();
		try {
			std::list<TopoLogicCore::Edge*> coreEdges;
			topoLogicVertex->m_pCoreVertex->Edges(coreEdges);

			List<Edge^>^ pEdges = gcnew List<Edge^>();
			List<Object^>^ pDynamoCurves = gcnew List<Object^>();
			for (std::list<TopoLogicCore::Edge*>::iterator coreEdgeIterator = coreEdges.begin();
				coreEdgeIterator != coreEdges.end();
				coreEdgeIterator++)
			{
				TopoLogicCore::Edge* pCoreEdge = *coreEdgeIterator;
				Edge^ pEdge = gcnew Edge(pCoreEdge);
				pEdges->Add(pEdge);
				pDynamoCurves->Add(pEdge->Geometry);
			}

			pDictionary->Add("TopoLogic Edges", pEdges);
			pDictionary->Add("Curves", pDynamoCurves);
		}
		catch (ArgumentNullException^)
		{
			throw gcnew ArgumentNullException("point");
		}
		return pDictionary;
	}

	Vertex::Vertex(Autodesk::DesignScript::Geometry::Point ^ pDynamoPoint)
		: Topology()
		, m_pCoreVertex(TopoLogicCore::Vertex::ByPoint(new Geom_CartesianPoint(gp_Pnt(pDynamoPoint->X, pDynamoPoint->Y, pDynamoPoint->Z))))
	{

	}

	Vertex::~Vertex()
	{
		delete m_pCoreVertex;
	}

	Object^ Vertex::Geometry::get()
	{
		gp_Pnt occtPoint = BRep_Tool::Pnt(TopoDS::Vertex(*m_pCoreVertex->GetOcctShape()));
		return Autodesk::DesignScript::Geometry::Point::ByCoordinates(occtPoint.X(), occtPoint.Y(), occtPoint.Z());
	}
}