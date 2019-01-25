#pragma once

#include <TopologyFactory.h>

namespace Topologic
{
	/// <summary>
	/// (private) A private factory class to create a Cell.
	/// </summary>
	ref class CellFactory : TopologyFactory
	{
	public:
		/// <summary>
		/// Creates a Cell from a TopologicCore layer Topology.
		/// </summary>
		/// <param name="kpTopology">A TopologicCore layer Topology</param>
		/// <returns name="Topology">The created Cell</returns>
		[IsVisibleInDynamoLibrary(false)]
		virtual Topology^ Create(const TopologicCore::TopologyPtr& kpTopology) override;
	};
}