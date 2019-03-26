#include "Graph.h"
#include "Vertex.h"
#include "Edge.h"
#include "Wire.h"
#include "Topology.h"

#include <TopologicCore/include/Vertex.h>
#include <TopologicCore/include/Edge.h>

using namespace System;

namespace Topologic
{
	Graph ^ Graph::ByTopology(
		Topologic::Topology ^ topology,
		bool direct,
		bool viaSharedTopologies,
		bool viaSharedApertures,
		bool toExteriorTopologies,
		bool toExteriorApertures)
	{
		TopologicCore::Topology::Ptr pCoreTopology = TopologicCore::Topology::Downcast<TopologicCore::Topology>(topology->GetCoreTopologicalQuery());
		TopologicCore::Graph::Ptr pCoreGraph = TopologicCore::Graph::ByTopology(
			pCoreTopology,
			direct,
			viaSharedTopologies,
			viaSharedApertures,
			toExteriorTopologies,
			toExteriorApertures);
		if (pCoreGraph == nullptr)
		{
			return nullptr;
		}
		return gcnew Graph(pCoreGraph);
	}

	List<Vertex^>^ Graph::VerticesAtCoordinates(double x, double y, double z, double tolerance)
	{
		TopologicCore::Graph::Ptr pCoreGraph = *m_pCoreGraph;

		std::list<TopologicCore::Vertex::Ptr> coreVertices;
			pCoreGraph->VerticesAtCoordinates(x, y, z, tolerance, coreVertices);

		List<Vertex^>^ vertices = gcnew List<Vertex^>();
		for (const TopologicCore::Vertex::Ptr& kpCoreVertex : coreVertices)
		{
			Vertex^ vertex = gcnew Vertex(kpCoreVertex);
			vertices->Add(vertex);
		}
		return vertices;
	}

	Graph ^ Graph::AddVertices(List<Vertex^>^ vertices, Nullable<double> tolerance)
	{
		TopologicCore::Graph::Ptr pCoreGraph = *m_pCoreGraph;
		TopologicCore::Graph::Ptr pCoreCopyGraph = std::make_shared<TopologicCore::Graph>(pCoreGraph.get());

		std::list<TopologicCore::Vertex::Ptr> coreVertices;
		for each(Vertex^ vertex in vertices)
		{
			TopologicCore::Vertex::Ptr pCoreVertex = TopologicCore::Topology::Downcast<TopologicCore::Vertex>(vertex->GetCoreTopologicalQuery());
			TopologicCore::Vertex::Ptr pCoreCopyVertex = TopologicCore::Topology::Downcast<TopologicCore::Vertex>(pCoreVertex->DeepCopy());
			coreVertices.push_back(pCoreCopyVertex);
		}

		if (tolerance.HasValue)
		{
			pCoreCopyGraph->AddVertices(coreVertices, true, tolerance.Value);
		}
		else
		{
			pCoreCopyGraph->AddVertices(coreVertices, false, 0.0);
		}

		return gcnew Graph(pCoreCopyGraph);
	}

	Graph ^ Graph::AddEdges(List<Edge^>^ edges, Nullable<double> tolerance)
	{
		TopologicCore::Graph::Ptr pCoreGraph = *m_pCoreGraph;
		TopologicCore::Graph::Ptr pCoreCopyGraph = std::make_shared<TopologicCore::Graph>(pCoreGraph.get());

		std::list<TopologicCore::Edge::Ptr> coreEdges;
		for each(Edge^ edge in edges)
		{
			TopologicCore::Edge::Ptr pCoreEdge = TopologicCore::Topology::Downcast<TopologicCore::Edge>(edge->GetCoreTopologicalQuery());
			TopologicCore::Edge::Ptr pCoreCopyEdge = TopologicCore::Topology::Downcast<TopologicCore::Edge>(pCoreEdge->DeepCopy());
			coreEdges.push_back(pCoreCopyEdge);
		}

		if (tolerance.HasValue)
		{
			pCoreCopyGraph->AddEdges(coreEdges, true, tolerance.Value);
		}
		else
		{
			pCoreCopyGraph->AddEdges(coreEdges, false, 0.0);
		}

		return gcnew Graph(pCoreCopyGraph);
	}

	List<Wire^>^ Graph::AllPaths(Vertex ^ startVertex, Vertex ^ endVertex, Nullable<int> timeLimitInSeconds)
	{
		TopologicCore::Vertex::Ptr pCoreStartVertex = TopologicCore::Topology::Downcast<TopologicCore::Vertex>(startVertex->GetCoreTopologicalQuery());
		TopologicCore::Vertex::Ptr pCoreEndVertex = TopologicCore::Topology::Downcast<TopologicCore::Vertex>(endVertex->GetCoreTopologicalQuery());

		std::list<TopologicCore::Wire::Ptr> corePaths;
		if (timeLimitInSeconds.HasValue)
		{
			(*m_pCoreGraph)->AllPaths(pCoreStartVertex, pCoreEndVertex, true, timeLimitInSeconds.Value, corePaths);
		}
		else
		{
			(*m_pCoreGraph)->AllPaths(pCoreStartVertex, pCoreEndVertex, false, 0, corePaths);
		}

		List<Wire^>^ paths = gcnew List<Wire^>();
		for (const TopologicCore::Wire::Ptr& rkPath : corePaths)
		{
			Wire^ path = safe_cast<Wire^>(Topologic::Topology::ByCoreTopology(rkPath));
			paths->Add(path);
		}

		return paths;
	}

	Wire ^ Graph::Path(Vertex ^ startVertex, Vertex ^ endVertex)
	{
		TopologicCore::Vertex::Ptr pCoreStartVertex = TopologicCore::Topology::Downcast<TopologicCore::Vertex>(startVertex->GetCoreTopologicalQuery());
		TopologicCore::Vertex::Ptr pCoreEndVertex = TopologicCore::Topology::Downcast<TopologicCore::Vertex>(endVertex->GetCoreTopologicalQuery());

		TopologicCore::Wire::Ptr corePath = (*m_pCoreGraph)->Path(pCoreStartVertex, pCoreEndVertex);

		Wire^ path = safe_cast<Wire^>(Topologic::Topology::ByCoreTopology(corePath));
		return path;
	}

	Wire ^ Graph::ShortestPath(Vertex ^ startVertex, Vertex ^ endVertex)
	{
		TopologicCore::Vertex::Ptr pCoreStartVertex = TopologicCore::Topology::Downcast<TopologicCore::Vertex>(startVertex->GetCoreTopologicalQuery());
		TopologicCore::Vertex::Ptr pCoreEndVertex = TopologicCore::Topology::Downcast<TopologicCore::Vertex>(endVertex->GetCoreTopologicalQuery());

		TopologicCore::Wire::Ptr corePath = (*m_pCoreGraph)->ShortestPath(pCoreStartVertex, pCoreEndVertex);

		Wire^ path = safe_cast<Wire^>(Topologic::Topology::ByCoreTopology(corePath));
		return path;
	}

	double Graph::Distance(Vertex ^ startVertex, Vertex ^ endVertex)
	{
		TopologicCore::Vertex::Ptr pCoreStartVertex = TopologicCore::Topology::Downcast<TopologicCore::Vertex>(startVertex->GetCoreTopologicalQuery());
		TopologicCore::Vertex::Ptr pCoreEndVertex = TopologicCore::Topology::Downcast<TopologicCore::Vertex>(endVertex->GetCoreTopologicalQuery());

		return (*m_pCoreGraph)->Distance(pCoreStartVertex, pCoreEndVertex);
	}

	bool Graph::IsErdoesGallai(List<int>^ sequence)
	{
		std::list<int> coreSequence;
		for each(int i in sequence)
		{
			coreSequence.push_back(i);
		}
		return (*m_pCoreGraph)->IsErdoesGallai(coreSequence);
	}

	Graph^  Graph::RemoveVertices(List<Vertex^>^ vertices)
	{
		TopologicCore::Graph::Ptr pCoreGraph = *m_pCoreGraph;
		TopologicCore::Graph::Ptr pCoreCopyGraph = std::make_shared<TopologicCore::Graph>(pCoreGraph.get());

		std::list<TopologicCore::Vertex::Ptr> coreVertices;
		for each(Vertex^ vertex in vertices)
		{
			TopologicCore::Vertex::Ptr pCoreVertex = TopologicCore::Topology::Downcast<TopologicCore::Vertex>(vertex->GetCoreTopologicalQuery());
			TopologicCore::Vertex::Ptr pCoreCopyVertex = TopologicCore::Topology::Downcast<TopologicCore::Vertex>(pCoreVertex->DeepCopy());
			coreVertices.push_back(pCoreCopyVertex);
		}

		pCoreCopyGraph->RemoveVertices(coreVertices);

		return gcnew Graph(pCoreCopyGraph);
	}

	Graph^ Graph::RemoveEdges(List<Edge^>^ edges)
	{
		TopologicCore::Graph::Ptr pCoreGraph = *m_pCoreGraph;
		TopologicCore::Graph::Ptr pCoreCopyGraph = std::make_shared<TopologicCore::Graph>(pCoreGraph.get());

		std::list<TopologicCore::Edge::Ptr> coreEdges;
		for each(Edge^ edge in edges)
		{
			TopologicCore::Edge::Ptr pCoreEdge = TopologicCore::Topology::Downcast<TopologicCore::Edge>(edge->GetCoreTopologicalQuery());
			TopologicCore::Edge::Ptr pCoreCopyEdge = TopologicCore::Topology::Downcast<TopologicCore::Edge>(pCoreEdge->DeepCopy());
			coreEdges.push_back(pCoreCopyEdge);
		}

		pCoreCopyGraph->RemoveEdges(coreEdges);

		return gcnew Graph(pCoreCopyGraph);
	}

	Edge ^ Graph::EdgeAtVertices(Vertex ^ vertex1, Vertex ^ vertex2, Nullable<double> tolerance)
	{
		TopologicCore::Vertex::Ptr pCoreVertex1 = TopologicCore::Topology::Downcast<TopologicCore::Vertex>(vertex1->GetCoreTopologicalQuery());
		TopologicCore::Vertex::Ptr pCoreVertex2 = TopologicCore::Topology::Downcast<TopologicCore::Vertex>(vertex2->GetCoreTopologicalQuery());

		TopologicCore::Graph::Ptr pCoreGraph = *m_pCoreGraph;
		TopologicCore::Edge::Ptr pCoreEdge = nullptr;
		if (tolerance.HasValue)
		{
			pCoreEdge = pCoreGraph->EdgeAtVertices(pCoreVertex1, pCoreVertex2, true, tolerance.Value);
		}
		else
		{
			pCoreEdge = pCoreGraph->EdgeAtVertices(pCoreVertex1, pCoreVertex2, false, 0.0);
		}

		if (pCoreEdge == nullptr)
		{
			return nullptr;
		}

		return gcnew Edge(pCoreEdge);
	}

	List<Edge^>^ Graph::IncidentEdges(Vertex ^ vertex, Nullable<double> tolerance)
	{
		TopologicCore::Vertex::Ptr pCoreVertex = TopologicCore::Topology::Downcast<TopologicCore::Vertex>(vertex->GetCoreTopologicalQuery());
		TopologicCore::Graph::Ptr pCoreGraph = *m_pCoreGraph;

		std::list<TopologicCore::Edge::Ptr> coreEdges;
		if (tolerance.HasValue)
		{
			(*m_pCoreGraph)->IncidentEdges(pCoreVertex, true, tolerance.Value, coreEdges);
		}
		else
		{
			(*m_pCoreGraph)->IncidentEdges(pCoreVertex, false, 0.0, coreEdges);
		}

		List<Edge^>^ edges = gcnew List<Edge^>();
		for (const TopologicCore::Edge::Ptr& kpCoreEdge : coreEdges)
		{
			Edge^ edge = gcnew Edge(kpCoreEdge);
			edges->Add(edge);
		}
		return edges;
	}

	Graph::Graph(const std::shared_ptr<TopologicCore::Graph>& kpCoreGraph)
		: m_pCoreGraph(kpCoreGraph != nullptr ? new TopologicCore::Graph::Ptr(kpCoreGraph) : throw gcnew Exception("A null graph was created."))
	{

	}

	Graph::~Graph()
	{
		delete m_pCoreGraph;
	}

	Topology^ Graph::Topology::get()
	{
		TopologicCore::Topology::Ptr coreTopology = (*m_pCoreGraph)->Topology();
		return Topologic::Topology::ByCoreTopology(coreTopology);
	}

	List<Vertex^>^ Graph::Vertices::get()
	{
		std::list<TopologicCore::Vertex::Ptr> coreVertices;
		(*m_pCoreGraph)->Vertices(coreVertices);

		List<Vertex^>^ vertices = gcnew List<Vertex^>();
		for(const TopologicCore::Vertex::Ptr& kpCoreVertex : coreVertices)
		{
			Vertex^ vertex = gcnew Vertex(kpCoreVertex);
			vertices->Add(vertex);
		}
		return vertices;
	}

	List<Vertex^>^ Graph::IsolatedVertices::get()
	{
		std::list<TopologicCore::Vertex::Ptr> coreIsolatedVertices;
		(*m_pCoreGraph)->IsolatedVertices(coreIsolatedVertices);

		List<Vertex^>^ isolatedVertices = gcnew List<Vertex^>();
		for (const TopologicCore::Vertex::Ptr& kpCoreVertex : coreIsolatedVertices)
		{
			Vertex^ vertex = gcnew Vertex(kpCoreVertex);
			isolatedVertices->Add(vertex);
		}
		return isolatedVertices;
	}

	List<Edge^>^ Graph::Edges::get()
	{
		std::list<TopologicCore::Edge::Ptr> coreEdges;
		(*m_pCoreGraph)->Edges(coreEdges);

		List<Edge^>^ edges = gcnew List<Edge^>();
		for (const TopologicCore::Edge::Ptr& kpCoreEdge : coreEdges)
		{
			Edge^ edge = gcnew Edge(kpCoreEdge);
			edges->Add(edge);
		}
		return edges;
	}

	int Graph::Degree(Vertex^ vertex)
	{
		TopologicCore::Vertex::Ptr pCoreVertex = TopologicCore::Topology::Downcast<TopologicCore::Vertex>(vertex->GetCoreTopologicalQuery());

		return (*m_pCoreGraph)->Degree(pCoreVertex);
	}

	List<Vertex^>^ Graph::AdjacentVertices(Vertex ^ vertex)
	{
		TopologicCore::Vertex::Ptr pCoreVertex = TopologicCore::Topology::Downcast<TopologicCore::Vertex>(vertex->GetCoreTopologicalQuery());
		std::list<TopologicCore::Vertex::Ptr> coreAdjacentVertices;
		(*m_pCoreGraph)->AdjacentVertices(pCoreVertex, coreAdjacentVertices);

		List<Vertex^>^ adjacentVertices = gcnew List<Vertex^>();
		for (const TopologicCore::Vertex::Ptr& kpCoreAdjacentVertex : coreAdjacentVertices)
		{
			Vertex^ adjacentVertex = gcnew Vertex(kpCoreAdjacentVertex);
			adjacentVertices->Add(adjacentVertex);
		}
		return adjacentVertices;
	}

	Graph ^ Graph::Connect(Vertex ^ vertex1, Vertex ^ vertex2, Nullable<double> tolerance)
	{
		TopologicCore::Vertex::Ptr pCoreVertex1 = TopologicCore::Topology::Downcast<TopologicCore::Vertex>(vertex1->GetCoreTopologicalQuery());
		TopologicCore::Vertex::Ptr pCoreVertex2 = TopologicCore::Topology::Downcast<TopologicCore::Vertex>(vertex2->GetCoreTopologicalQuery());
		TopologicCore::Graph::Ptr pCoreGraph = *m_pCoreGraph;

		/*TopologicCore::Vertex::Ptr pCoreCopyVertex1 = TopologicCore::Topology::Downcast<TopologicCore::Vertex>(pCoreVertex1->DeepCopy());
		TopologicCore::Vertex::Ptr pCoreCopyVertex2 = TopologicCore::Topology::Downcast<TopologicCore::Vertex>(pCoreVertex2->DeepCopy());*/
		TopologicCore::Graph::Ptr pCoreCopyGraph = std::make_shared<TopologicCore::Graph>(pCoreGraph.get());

		//pCoreCopyGraph->Connect(pCoreCopyVertex1, pCoreCopyVertex2);
		if (tolerance.HasValue)
		{
			pCoreCopyGraph->Connect(pCoreVertex1, pCoreVertex2, true, tolerance.Value);
		}
		else
		{
			pCoreCopyGraph->Connect(pCoreVertex1, pCoreVertex2, false, 0.0);
		}

		return gcnew Graph(pCoreCopyGraph);
	}

	bool Graph::ContainsVertex(Vertex ^ vertex, Nullable<double> tolerance)
	{
		TopologicCore::Vertex::Ptr pCoreVertex = TopologicCore::Topology::Downcast<TopologicCore::Vertex>(vertex->GetCoreTopologicalQuery());

		if (tolerance.HasValue)
		{
			return (*m_pCoreGraph)->ContainsVertex(pCoreVertex, true, tolerance.Value);
		}
		else
		{
			return (*m_pCoreGraph)->ContainsVertex(pCoreVertex, false, 0.0);
		}
	}

	bool Graph::ContainsEdge(Edge ^ edge, Nullable<double> tolerance)
	{
		TopologicCore::Edge::Ptr pCoreEdge = TopologicCore::Topology::Downcast<TopologicCore::Edge>(edge->GetCoreTopologicalQuery());

		if (tolerance.HasValue)
		{
			return (*m_pCoreGraph)->ContainsEdge(pCoreEdge, true, tolerance.Value);
		}
		else
		{
			return (*m_pCoreGraph)->ContainsEdge(pCoreEdge, false, 0.0);
		}

	}

	List<int>^ Graph::DegreeSequence::get()
	{
		std::list<int> coreDegreeSequence;
		(*m_pCoreGraph)->DegreeSequence(coreDegreeSequence);
		List<int>^ degreeSequence = gcnew List<int>();
		for (const int kDegree : coreDegreeSequence)
		{
			degreeSequence->Add(kDegree);
		}
		return degreeSequence;
	}

	double Graph::Density::get()
	{
		return (*m_pCoreGraph)->Density();
	}

	bool Graph::IsComplete::get()
	{
		return (*m_pCoreGraph)->IsComplete();
	}

	int Graph::MinimumDelta::get()
	{
		return (*m_pCoreGraph)->MinimumDelta();
	}

	int Graph::MaximumDelta::get()
	{
		return (*m_pCoreGraph)->MaximumDelta();
	}

	int Graph::Diameter::get()
	{
		return (*m_pCoreGraph)->Diameter();
	}

}