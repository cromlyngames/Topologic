// This file is part of Topologic software library.
// Copyright(C) 2019, Cardiff University and University College London
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <TopologicCore/include/TopologicalQuery.h>

namespace Topologic
{
	/// <summary>
	/// TopologicalQuery is the base class for Topology and Context classes. 
	/// </summary>

	public ref class TopologicalQuery abstract
	{
	public protected:
		virtual std::shared_ptr<TopologicCore::TopologicalQuery> GetCoreTopologicalQuery() = 0;

	protected:
		TopologicalQuery() {}
		virtual ~TopologicalQuery() {}
	};
}