#include <Shell.h>
#include <GlobalCluster.h>
#include <Cell.h>
#include <Vertex.h>
#include <Edge.h>
#include <Wire.h>
#include <Face.h>

#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepCheck_Shell.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>
#include <Geom_Surface.hxx>
#include <ShapeFix_ShapeTolerance.hxx>
#include <ShapeFix_Shell.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Wire.hxx>
#include <TopTools_MapOfShape.hxx>

// ShapeOp
#include <Constraint.h>
#include <Solver.h>


#include <assert.h>

namespace TopoLogicCore
{
	void Shell::Cells(std::list<std::shared_ptr<Cell>>& rCells) const
	{
		UpwardNavigation(rCells);
	}

	void Shell::Edges(std::list<std::shared_ptr<Edge>>& rEdges) const
	{
		DownwardNavigation(rEdges);
	}

	void Shell::Wires(std::list<std::shared_ptr<Wire>>& rWires) const
	{
		DownwardNavigation(rWires);
	}

	void Shell::Faces(std::list<std::shared_ptr<Face>>& rFaces) const
	{
		DownwardNavigation(rFaces);
	}

	bool Shell::IsClosed() const
	{
		BRepCheck_Shell occtBrepCheckShell(TopoDS::Shell(*GetOcctShape()));
		return (occtBrepCheckShell.Closed() == BRepCheck_NoError);
	}

	void Shell::Vertices(std::list<std::shared_ptr<Vertex>>& rVertices) const
	{
		DownwardNavigation(rVertices);
	}

	std::shared_ptr<Shell> Shell::ByFaces(const std::list<std::shared_ptr<Face>>& rkFaces)
	{
		if (rkFaces.empty())
		{
			throw std::exception("No face is passed.");
		}

		BRepBuilderAPI_Sewing occtSewing;
		for(std::list<std::shared_ptr<Face>>::const_iterator kFaceIterator = rkFaces.begin();
			kFaceIterator != rkFaces.end();
			kFaceIterator++)
		{
			const std::shared_ptr<Face>& kpFace = *kFaceIterator;
			occtSewing.Add(*kpFace->GetOcctShape());
		}

		occtSewing.Perform();
		std::shared_ptr<Shell> pShell = std::make_shared<Shell>(TopoDS::Shell(occtSewing.SewedShape()));
		for (std::list<std::shared_ptr<Face>>::const_iterator kFaceIterator = rkFaces.begin();
			kFaceIterator != rkFaces.end();
			kFaceIterator++)
		{
			const std::shared_ptr<Face>& kpFace = *kFaceIterator;
			kpFace->AddIngredientTo(pShell);
		}

		return pShell;
	}

	std::shared_ptr<Shell> Shell::ByVerticesFaceIndices(const std::vector<std::shared_ptr<Vertex>>& rkVertices, const std::list<std::list<int>>& rkFaceIndices)
	{
		std::list<std::shared_ptr<Face>> faces;
		for(std::list<std::list<int>>::const_iterator kFaceIndexIterator = rkFaceIndices.begin();
			kFaceIndexIterator != rkFaceIndices.end();
			kFaceIndexIterator++)
		{
			const std::list<int>& rkVertexIndices = *kFaceIndexIterator;

			BRepBuilderAPI_MakeWire occtMakeWire;

			std::list<int>::const_iterator kSecondFromLastVertexIterator = --rkVertexIndices.end();
			for(std::list<int>::const_iterator kVertexIterator = rkVertexIndices.begin();
				kVertexIterator != kSecondFromLastVertexIterator;
				kVertexIterator++)
			{
				int vertexIndex = *kVertexIterator;

				std::list<int>::const_iterator kNextVertexIterator = kVertexIterator;
				kNextVertexIterator++;
				int nextVertexIndex = *kNextVertexIterator;

				occtMakeWire.Add(BRepBuilderAPI_MakeEdge(
					TopoDS::Vertex(*rkVertices[vertexIndex]->GetOcctShape()),
					TopoDS::Vertex(*rkVertices[nextVertexIndex]->GetOcctShape())
				));
			}
			occtMakeWire.Add(BRepBuilderAPI_MakeEdge(
				TopoDS::Vertex(*rkVertices[*--rkVertexIndices.end()]->GetOcctShape()),
				TopoDS::Vertex(*rkVertices[*rkVertexIndices.begin()]->GetOcctShape())
			));

			TopoDS_Wire pOcctWire(occtMakeWire);
			BRepBuilderAPI_MakeFace occtMakeFace(pOcctWire);
			faces.push_back(std::make_shared<Face>(occtMakeFace));
		}
		std::shared_ptr<Shell> pShell = ByFaces(faces);

		// Only add the vertices; the faces are dealt with in ByFaces()
		for (std::vector<std::shared_ptr<Vertex>>::const_iterator kVertexIterator = rkVertices.begin();
			kVertexIterator != rkVertices.end();
			kVertexIterator++)
		{
			const std::shared_ptr<Vertex>& kpVertex = *kVertexIterator;
			kpVertex->AddIngredientTo(pShell);
		}
		return pShell;
	}

	std::shared_ptr<Shell> Shell::ByFacePlanarization(
		const std::shared_ptr<Face>& kpFace,
		const int kIteration,
		const int kNumUPanels,
		const int kNumVPanels,
		const double kTolerance,
		const bool isCapped)
	{
		// Check if the topology is a face, a shell, a cell, or a cell complex.
		//if (!(rTopology.GetType() == TOPOLOGY_FACE ||
		//	rTopology.GetType() == TOPOLOGY_SHELL ||
		//	rTopology.GetType() == TOPOLOGY_CELL ||
		//	rTopology.GetType() == TOPOLOGY_CELLCOMPLEX))
		//{
		//	std::string errorMessage = std::string("The topology is a ") + rTopology.GetTypeAsString() + std::string(". It needs to be a face, a shell, a cell, or a cell complex.");
		//	throw std::exception(errorMessage.c_str());
		//}

		//// If the topology is a face, it has to be the same as the second argument.
		//if (rTopology.GetType() == TOPOLOGY_FACE && rTopology.GetOcctShape()->IsSame(*rFace.GetOcctShape()))
		//{
		//	std::string errorMessage = std::string("Faces in the first and second arguments need to be the same");
		//	throw std::exception(errorMessage.c_str());
		//}

		// Check if rFace part of rTopology
		/*TopTools_IndexedDataMapOfShapeListOfShape occtParentsMap;
		TopoDS_Shape* pOcctTopology = rTopology.GetOcctShape();
		TopoDS_Shape* pOcctFace = rFace.GetOcctShape();
		TopExp::MapShapesAndUniqueAncestors(*pOcctTopology, TopAbs_FACE, pOcctTopology->ShapeType(), occtParentsMap);
		const TopTools_ListOfShape& rOcctParents = occtParentsMap.FindFromKey(*pOcctFace);

		bool isParent = false;
		for (TopTools_ListOfShape::const_iterator kParentIterator = rOcctParents.cbegin();
			kParentIterator != rOcctParents.cend() && !isParent;
			kParentIterator++)
		{
			const TopoDS_Shape& rOcctParent = *kParentIterator;
			if (rOcctParent.IsSame(*pOcctTopology))
			{
				isParent = true;
			}
		}

		if (!isParent)
		{
			std::string errorMessage = std::string("The topology is not a parent of the face.");
			throw std::exception(errorMessage.c_str());
		}*/

		// Check that kIteration, kNumUPanels, and kNumVPanels are 1 or more.
		if (kIteration < 1)
		{
			std::string errorMessage = std::string("The number of iteration must be larger than 0.");
			throw std::exception(errorMessage.c_str());
		}
		if (kNumUPanels < 1)
		{
			std::string errorMessage = std::string("The number of u-panels must be larger than 0.");
			throw std::exception(errorMessage.c_str());
		}
		if (kNumVPanels < 1)
		{
			std::string errorMessage = std::string("The number of v-panels must be larger than 0.");
			throw std::exception(errorMessage.c_str());
		}
		if (kNumVPanels <= 0.0)
		{
			std::string errorMessage = std::string("The tolerance must have a positive value");
			throw std::exception(errorMessage.c_str());
		}

		//// Get the list of topology's faces that are neither:
		//// - The input face
		//// - The input face's neighbour
		//BOPCol_ListOfShape occtUnaffectedFaces;
		//BOPCol_DataMapOfShapeListOfShape occtMapNeighbouringFacesEdges;
		//TopExp_Explorer occtExplorer;
		//for (occtExplorer.Init(*pOcctTopology, TopAbs_FACE); occtExplorer.More(); occtExplorer.Next())
		//{
		//	const TopoDS_Shape& rkCurrentFace = occtExplorer.Current();
		//	if (pOcctFace->IsSame(rkCurrentFace))
		//	{
		//		continue;
		//	}

		//	// Check if adjacent
		//	TopExp_Explorer occtExplorer1;
		//	bool isAdjacent = false;
		//	for (occtExplorer1.Init(*pOcctFace, TopAbs_EDGE); occtExplorer1.More() && !isAdjacent; occtExplorer1.Next())
		//	{
		//		const TopoDS_Shape& rkOcctEdge1 = occtExplorer1.Current();
		//		TopExp_Explorer occtExplorer2;
		//		for (occtExplorer2.Init(rkCurrentFace, TopAbs_EDGE); occtExplorer2.More() && !isAdjacent; occtExplorer2.Next())
		//		{
		//			const TopoDS_Shape& rkOcctEdge2 = occtExplorer2.Current();

		//			// Contains is called as the two faces may be adjacent through more than edges.
		//			if (rkOcctEdge1.IsSame(rkOcctEdge2))
		//			{
		//				// Find the shared edges
		//				try {
		//					BOPCol_ListOfShape& rOcctEdges = occtMapNeighbouringFacesEdges.ChangeFind(rkCurrentFace);
		//					rOcctEdges.Append(rkOcctEdge2);
		//				}
		//				catch (Standard_NoSuchObject&)
		//				{
		//					BOPCol_ListOfShape occtEdges;
		//					occtEdges.Append(rkOcctEdge1);
		//					occtMapNeighbouringFacesEdges.Bind(rkCurrentFace, occtEdges);
		//				}
		//			}
		//		}
		//	}

		//	// Otherwise
		//	occtUnaffectedFaces.Append(rkCurrentFace);
		//}

		// Subdivide the face in the UV space and find the points
		Handle(Geom_Surface) pOcctSurface = kpFace->Surface();
		double uMin = 0.0, uMax = 0.0, vMin = 0.0, vMax = 0.0;
		pOcctSurface->Bounds(uMin, uMax, vMin, vMax);
		double uRange = uMax - uMin;
		double vRange = vMax - vMin;
		double dU = uRange / (double)kNumUPanels;
		double dV = vRange / (double)kNumVPanels;
		bool isUClosed = pOcctSurface->IsUClosed();
		int numUPoints = kNumUPanels;
		if (!isUClosed)
		{
			numUPoints += 1;
		}
		bool isVClosed = pOcctSurface->IsVClosed();
		int numVPoints = kNumVPanels;
		if (!isVClosed)
		{
			numVPoints += 1;
		}

		std::vector<double> uSubdivision;
		uSubdivision.push_back(uMin);
		for (int i = 1; i < kNumUPanels; ++i) // 1 to kNumUPanels - 1 because 1) it is the vertices being processed,
											  //                              2) uMin and uMax already added
		{
			uSubdivision.push_back(uMin + dU * i);
		}
		uSubdivision.push_back(uMax);

		std::vector<double> vSubdivision;
		vSubdivision.push_back(vMin);
		for (int i = 1; i < kNumVPanels; ++i) // 1 to kNumVPanels - 1 because 1) it is the vertices being processed,
											  //                              2) vMin and vMax already added
		{
			vSubdivision.push_back(vMin + dV * i);
		}
		vSubdivision.push_back(vMax);

		// Initialise ShapeOp matrix
		int numOfPoints = numUPoints * numVPoints;
		ShapeOp::Matrix3X shapeOpMatrix(3, numOfPoints); //column major
		int i = 0;
		std::vector<double>::const_iterator endUIterator = uSubdivision.end();
		if (isUClosed) 
		{
			endUIterator--;
		}
		std::vector<double>::const_iterator endVIterator = vSubdivision.end();
		if (isVClosed)
		{
			endVIterator--;
		}
		for (std::vector<double>::const_iterator uIterator = uSubdivision.begin();
			uIterator != endUIterator;
			uIterator++)
		{
			const double& rkU = *uIterator;
			for (std::vector<double>::const_iterator vIterator = vSubdivision.begin();
				vIterator != endVIterator;
				vIterator++)
			{
				const double& rkV = *vIterator;
				gp_Pnt occtPoint = pOcctSurface->Value(rkU, rkV);
				shapeOpMatrix(0, i) = occtPoint.X();
				shapeOpMatrix(1, i) = occtPoint.Y();
				shapeOpMatrix(2, i) = occtPoint.Z();

				++i;
			}
		}

		// Solve ShapeOp, including applying constraints.
		ShapeOp::Solver shapeOpSolver;
		shapeOpSolver.setPoints(shapeOpMatrix);
		ShapeOp::Scalar weight = 1.0;

		// Planarity constraint to the panels
		// i and j are just used to iterate, the actual indices are the u and v variables.
		for (int i = 0; i < kNumUPanels; ++i)
		{
			for (int j = 0; j < kNumVPanels; ++j)
			{
				// Default values
				int minU = i;		int minV = j;
				int maxU = i + 1;	int maxV = j + 1;
				
				// Border values
				if (isUClosed && i == kNumUPanels - 1)
				{
					maxU = 0;
				}
				if (isVClosed && j == kNumVPanels - 1)
				{
					maxV = 0;
				}

				std::vector<int> vertexIDs;
				vertexIDs.push_back(minU * kNumVPanels + minV);
				vertexIDs.push_back(minU * kNumVPanels + maxV);
				vertexIDs.push_back(maxU * kNumVPanels + maxV);
				vertexIDs.push_back(maxU * kNumVPanels + maxV);
				auto shapeOpConstraint = std::make_shared<ShapeOp::PlaneConstraint>(vertexIDs, weight, shapeOpSolver.getPoints());
				shapeOpSolver.addConstraint(shapeOpConstraint);
			}
		}

		// Closeness constraints to the vertices on the edges along the u-axis
		for (int i = 0; i < numUPoints; ++i)
		{
			{
				std::vector<int> vertexIDs;
				vertexIDs.push_back(i * numVPoints);
				auto shapeOpConstraint = std::make_shared<ShapeOp::ClosenessConstraint>(vertexIDs, weight, shapeOpSolver.getPoints());
				shapeOpSolver.addConstraint(shapeOpConstraint);
			}

			if(!isVClosed)
			{
				std::vector<int> vertexIDs;
				vertexIDs.push_back((i + 1) * numVPoints - 1);
				auto shapeOpConstraint = std::make_shared<ShapeOp::ClosenessConstraint>(vertexIDs, weight, shapeOpSolver.getPoints());
				shapeOpSolver.addConstraint(shapeOpConstraint);
			}
		}

		// Closeness constraints to the vertices on the edge along the v-axis
		for (int j = 0; j < numVPoints; ++j)
		{
			{
				std::vector<int> vertexIDs;
				vertexIDs.push_back(j);
				auto shapeOpConstraint = std::make_shared<ShapeOp::ClosenessConstraint>(vertexIDs, weight, shapeOpSolver.getPoints());
				shapeOpSolver.addConstraint(shapeOpConstraint);
			}

			if(!isUClosed)
			{
				std::vector<int> vertexIDs;
				vertexIDs.push_back((numUPoints-1) * numVPoints + j);
				auto shapeOpConstraint = std::make_shared<ShapeOp::ClosenessConstraint>(vertexIDs, weight, shapeOpSolver.getPoints());
				shapeOpSolver.addConstraint(shapeOpConstraint);
			}
		}

		// Closeness constraints to the vertices not on the surface's edges
		for (int i = 1; i < kNumUPanels; ++i)
		{
			for (int j = 1; j < kNumVPanels; ++j)
			{
				std::vector<int> vertexIDs;
				vertexIDs.push_back(i * numVPoints + j);
				auto shapeOpConstraint = std::make_shared<ShapeOp::ClosenessConstraint>(vertexIDs, 0.1*weight, shapeOpSolver.getPoints());
				shapeOpSolver.addConstraint(shapeOpConstraint);
			}
		}

		bool initialisationResult = shapeOpSolver.initialize();
		if (!initialisationResult)
			throw std::exception("Failed to initialize solver.");
		bool solveResult = shapeOpSolver.solve(kIteration);
		if (!solveResult)
			throw std::exception("Failed to solve.");
		const ShapeOp::Matrix3X& rkShapeOpResult = shapeOpSolver.getPoints();

		// Create faces from rkShapeOpResult
		TopTools_ListOfShape borderEdges[4]; // 0 = uMin, 1 = uMax, 2 = vMin, 3 = vMax
		                                   // Use array for iteration

		BRepBuilderAPI_Sewing occtSewing;
		for (int i = 0; i < kNumUPanels; ++i)
		{
			for (int j = 0; j < kNumVPanels; ++j)
			{
				// Default values
				int minU = i;		int minV = j;
				int maxU = i + 1;	int maxV = j + 1;

				// Border values
				if (isUClosed && i == kNumUPanels - 1)
				{
					maxU = 0;
				}
				if (isVClosed && j == kNumVPanels - 1)
				{
					maxV = 0;
				}

				int index1 = minU * numVPoints + minV;
				Handle(Geom_Point) pOcctPoint1 = new Geom_CartesianPoint(rkShapeOpResult(0, index1), rkShapeOpResult(1, index1), rkShapeOpResult(2, index1));

				int index2 = minU * numVPoints + maxV;
				Handle(Geom_Point) pOcctPoint2 = new Geom_CartesianPoint(rkShapeOpResult(0, index2), rkShapeOpResult(1, index2), rkShapeOpResult(2, index2));

				int index3 = maxU * numVPoints + maxV;
				Handle(Geom_Point) pOcctPoint3 = new Geom_CartesianPoint(rkShapeOpResult(0, index3), rkShapeOpResult(1, index3), rkShapeOpResult(2, index3));

				int index4 = maxU * numVPoints + minV;
				Handle(Geom_Point) pOcctPoint4 = new Geom_CartesianPoint(rkShapeOpResult(0, index4), rkShapeOpResult(1, index4), rkShapeOpResult(2, index4));

				BRepBuilderAPI_MakeVertex occtMakeVertex1(pOcctPoint1->Pnt());
				BRepBuilderAPI_MakeVertex occtMakeVertex2(pOcctPoint2->Pnt());
				BRepBuilderAPI_MakeVertex occtMakeVertex3(pOcctPoint3->Pnt());
				BRepBuilderAPI_MakeVertex occtMakeVertex4(pOcctPoint4->Pnt());

				BRepBuilderAPI_MakeEdge occtMakeEdge12(occtMakeVertex1.Vertex(), occtMakeVertex2.Vertex());
				BRepBuilderAPI_MakeEdge occtMakeEdge23(occtMakeVertex2.Vertex(), occtMakeVertex3.Vertex());
				BRepBuilderAPI_MakeEdge occtMakeEdge34(occtMakeVertex3.Vertex(), occtMakeVertex4.Vertex());
				BRepBuilderAPI_MakeEdge occtMakeEdge41(occtMakeVertex4.Vertex(), occtMakeVertex1.Vertex());

				const TopoDS_Shape& rkEdge12 = occtMakeEdge12.Shape();
				const TopoDS_Shape& rkEdge23 = occtMakeEdge23.Shape();
				const TopoDS_Shape& rkEdge34 = occtMakeEdge34.Shape();
				const TopoDS_Shape& rkEdge41 = occtMakeEdge41.Shape();

				TopTools_ListOfShape occtEdges;
				occtEdges.Append(rkEdge12);
				occtEdges.Append(rkEdge23);
				occtEdges.Append(rkEdge34);
				occtEdges.Append(rkEdge41);

				BRepBuilderAPI_MakeWire occtMakeWire;
				occtMakeWire.Add(occtEdges);

				const TopoDS_Wire& rkPanelWire = occtMakeWire.Wire();
				ShapeFix_ShapeTolerance occtShapeTolerance;
				occtShapeTolerance.SetTolerance(rkPanelWire, kTolerance, TopAbs_WIRE);
				BRepBuilderAPI_MakeFace occtMakeFace(rkPanelWire);
				occtSewing.Add(occtMakeFace.Face());

				if(isCapped)
				{
					if (!isUClosed && j == 0) // Use j == 0 to do this only once
					{
						if (i == 0) // Store the edge rkEdge12 to uMin edges 
						{
							borderEdges[0].Append(rkEdge12);
						}
						else if (i == kNumUPanels - 1) // Store the edge rkEdge34 to uMax edges 
						{
							borderEdges[1].Append(rkEdge34);
						}
					}

					if (!isVClosed)
					{
						if (j == 0) // Store the edge rkEdge41 to vMin edges 
						{
							borderEdges[2].Append(rkEdge41);
						}
						else if (j == kNumVPanels - 1) // Store the edge rkEdge23 to vMax edges 
						{
							borderEdges[3].Append(rkEdge23);
						}
					}
				}
			}
		}

		if(isCapped)
		{
			for (int i = 0; i < 4; ++i)
			{
				if(!borderEdges[i].IsEmpty())
				{
					// Make wire from the edges.
					BRepBuilderAPI_MakeWire occtMakeWire;
					occtMakeWire.Add(borderEdges[i]);

					const TopoDS_Wire& rkOuterWire = occtMakeWire.Wire();
					ShapeFix_ShapeTolerance occtShapeTolerance;
					occtShapeTolerance.SetTolerance(rkOuterWire, kTolerance, TopAbs_WIRE);
					BRepBuilderAPI_MakeFace occtMakeFace(TopoDS::Wire(occtMakeWire.Shape()));
					occtSewing.Add(occtMakeFace.Face());
				}
			}
		}

		occtSewing.Perform();


		// Construct the hole wires

		// Reconstruct the edges

		// Reconstruct the shape
		return std::make_shared<Shell>(TopoDS::Shell(occtSewing.SewedShape()));
	}

	std::shared_ptr<Shell> Shell::ByLoft(const std::list<std::shared_ptr<Wire>>& rkWires)
	{
		BRepOffsetAPI_ThruSections occtLoft;
		std::for_each(rkWires.begin(), rkWires.end(),
			[&occtLoft](const std::shared_ptr<Wire>& kpWire)
		{
			occtLoft.AddWire(TopoDS::Wire(*kpWire->GetOcctShape()));
		});
		occtLoft.Build();
		return std::make_shared<Shell>(TopoDS::Shell(occtLoft.Shape()));
	}

	std::shared_ptr<TopoDS_Shape> Shell::GetOcctShape() const
	{
		assert(m_pOcctShell != nullptr && "Shell::m_pOcctShell is null.");
		if (m_pOcctShell == nullptr)
		{
			throw std::exception("Shell::m_pOcctShell is null.");
		}

		return m_pOcctShell;
	}

	void Shell::Geometry(std::list<Handle(Geom_Geometry)>& rOcctGeometries) const
	{
		// Returns a list of faces
		std::list<std::shared_ptr<Face>> faces;
		Faces(faces);
		for (std::list<std::shared_ptr<Face>>::const_iterator kFaceIterator = faces.begin();
			kFaceIterator != faces.end();
			kFaceIterator++)
		{
			const std::shared_ptr<Face>& kpFace = *kFaceIterator;
			rOcctGeometries.push_back(kpFace->Surface());
		}
	}

	Shell::Shell(const TopoDS_Shell& rkOcctShell)
		: Topology(2)
		, m_pOcctShell(nullptr)
	{
		ShapeFix_Shell occtShapeFixShell(rkOcctShell);
		occtShapeFixShell.Perform();
		m_pOcctShell = std::make_shared<TopoDS_Shell>(occtShapeFixShell.Shell());
		GlobalCluster::GetInstance().Add(this);
	}

	Shell::~Shell()
	{
		GlobalCluster::GetInstance().Remove(this);
	}
}
