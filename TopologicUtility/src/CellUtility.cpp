#include "CellUtility.h"

#include <TopologicCore/include/Cell.h>
#include <TopologicCore/include/Face.h>
#include <TopologicCore/include/GlobalCluster.h>

#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <BRepGProp.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <GProp_GProps.hxx>
#include <Message_ProgressIndicator.hxx>
#include <ShapeFix_Solid.hxx>
#include <TopoDS.hxx>

namespace TopologicUtility
{
	TopologicCore::Cell::Ptr CellUtility::ByVerticesFaceIndices(const std::vector<TopologicCore::Vertex::Ptr>& rkVertices, const std::list<std::list<int>>& rkFaceIndices)
	{
		if (rkVertices.empty())
		{
			throw std::exception("No vertex is passed.");
		}

		std::list<TopologicCore::Face::Ptr> faces;
		for (const std::list<int>& rkVertexIndices : rkFaceIndices)
		{
			BRepBuilderAPI_MakeWire occtMakeWire;

			std::list<int>::const_iterator kSecondFromLastVertexIterator = --rkVertexIndices.end();
			for (std::list<int>::const_iterator kVertexIterator = rkVertexIndices.begin();
				kVertexIterator != kSecondFromLastVertexIterator;
				kVertexIterator++)
			{
				int vertexIndex = *kVertexIterator;

				std::list<int>::const_iterator kNextVertexIterator = kVertexIterator;
				kNextVertexIterator++;
				int nextVertexIndex = *kNextVertexIterator;

				occtMakeWire.Add(BRepBuilderAPI_MakeEdge(
					rkVertices[vertexIndex]->GetOcctVertex(),
					rkVertices[nextVertexIndex]->GetOcctVertex())
				);
			}
			occtMakeWire.Add(BRepBuilderAPI_MakeEdge(
				rkVertices[*--rkVertexIndices.end()]->GetOcctVertex(),
				rkVertices[*rkVertexIndices.begin()]->GetOcctVertex())
			);

			const TopoDS_Wire& rkOcctWire = occtMakeWire.Wire();
			BRepBuilderAPI_MakeFace occtMakeFace(rkOcctWire);
			faces.push_back(std::make_shared<TopologicCore::Face>(occtMakeFace));
		}
		TopologicCore::Cell::Ptr pCell = TopologicCore::Cell::ByFaces(faces);

		return pCell;
	}


	TopologicCore::Cell::Ptr CellUtility::ByLoft(const std::list<TopologicCore::Wire::Ptr>& rkWires)
	{
		BRepOffsetAPI_ThruSections occtLoft(true);
		for (const TopologicCore::Wire::Ptr& kpWire : rkWires)
		{
			occtLoft.AddWire(kpWire->GetOcctWire());
		};
		try {
			occtLoft.Build();
		}
		catch (...)
		{
			throw std::exception("Loft error");
		}
		return std::make_shared<TopologicCore::Cell>(TopoDS::Solid(occtLoft.Shape()));
	}

	TopologicCore::Cell::Ptr CellUtility::ByCuboid(
		const double kCentroidX, const double kCentroidY, const double kCentroidZ,
		const double kXDimension, const double kYDimension, const double kZDimension)
	{
		gp_Pnt occtLowCorner(
			kCentroidX - kXDimension / 2.0,
			kCentroidY - kYDimension / 2.0,
			kCentroidZ - kZDimension / 2.0
		);
		BRepPrimAPI_MakeBox occtMakeBox(occtLowCorner, kXDimension, kYDimension, kZDimension);
		occtMakeBox.Build();

		TopologicCore::Cell::Ptr pCell = std::make_shared<TopologicCore::Cell>(occtMakeBox.Solid());
		TopologicCore::GlobalCluster::GetInstance().AddTopology(pCell->GetOcctSolid());
		return pCell;
	}

	TopologicCore::Cell::Ptr CellUtility::BySphere(const double kCenterX, const double kCenterY, const double kCenterZ, const double kRadius)
	{
		BRepPrimAPI_MakeSphere occtMakeSphere(gp_Pnt(kCenterX, kCenterY, kCenterZ), kRadius);

		TopologicCore::Cell::Ptr pCell = std::make_shared<TopologicCore::Cell>(occtMakeSphere.Solid());
		TopologicCore::GlobalCluster::GetInstance().AddTopology(pCell->GetOcctSolid());
		return pCell;
	}

	TopologicCore::Cell::Ptr CellUtility::ByCylinder(const double kReferencePointX, const double kReferencePointY, const double kReferencePointZ,
		const double kNormalX, const double kNormalY, const double kNormalZ,
		const double kXDirectionX, const double kXDirectionY, const double kDirectionZ,
		const double kRadius, const double kHeight)
	{
		BRepPrimAPI_MakeCylinder occtMakeCylinder(
			gp_Ax2(
				gp_Pnt(kReferencePointX, kReferencePointY, kReferencePointZ),
				gp_Dir(kNormalX, kNormalY, kNormalZ),
				gp_Dir(kXDirectionX, kXDirectionY, kDirectionZ)),
			kRadius, kHeight);
		TopologicCore::Cell::Ptr pCell = std::make_shared<TopologicCore::Cell>(occtMakeCylinder.Solid());
		TopologicCore::GlobalCluster::GetInstance().AddTopology(pCell->GetOcctSolid());
		return pCell;
	}

	std::shared_ptr<TopologicCore::Cell> CellUtility::ByCone(
		const double kReferencePointX, const double kReferencePointY, const double kReferencePointZ,
		const double kNormalX, const double kNormalY, const double kNormalZ,
		const double kXDirectionX, const double kXDirectionY, const double kDirectionZ,
		const double kRadius1, const double kRadius2, const double kHeight)
	{
		BRepPrimAPI_MakeCone occtMakeCone(
			gp_Ax2(
				gp_Pnt(kReferencePointX, kReferencePointY, kReferencePointZ),
				gp_Dir(kNormalX, kNormalY, kNormalZ),
				gp_Dir(kXDirectionX, kXDirectionY, kDirectionZ)),
			kRadius1, kRadius2, kHeight);
		TopologicCore::Cell::Ptr pCell = std::make_shared<TopologicCore::Cell>(occtMakeCone.Solid());
		TopologicCore::GlobalCluster::GetInstance().AddTopology(pCell->GetOcctSolid());
		return pCell;
	}

	double CellUtility::Volume(const TopologicCore::Cell::Ptr & kpCell)
	{
		GProp_GProps occtShapeProperties;
		BRepGProp::VolumeProperties(kpCell->GetOcctShape(), occtShapeProperties);
		return occtShapeProperties.Mass();
	}

	bool CellUtility::DoesContain(const TopologicCore::Cell::Ptr & kpCell, const TopologicCore::Vertex::Ptr & kpVertex)
	{
		ShapeFix_Solid occtSolidFix(kpCell->GetOcctSolid());
		occtSolidFix.Perform();
		BRepClass3d_SolidClassifier occtSolidClassifier(occtSolidFix.Solid(), kpVertex->Point()->Pnt(), Precision::Confusion());
		TopAbs_State occtState = occtSolidClassifier.State();
		return (occtState == TopAbs_IN || occtState == TopAbs_ON);
	}

}