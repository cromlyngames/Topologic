#pragma once

using namespace System;
using namespace System::Collections::Generic;
using namespace Autodesk::DesignScript::Runtime;

namespace TopologicEnergy
{
	ref class EnergyModel;
	ref class EnergySimulation;

	public ref class SimulationResult
	{
	public:

		/// <summary>
		/// 
		/// </summary>
		/// <param name="energySimulation"></param>
		/// <param name="EPReportName"></param>
		/// <param name="EPReportForString"></param>
		/// <param name="EPTableName"></param>
		/// <param name="EPColumnName"></param>
		/// <param name="EPColumnName"></param>
		/// <param name="EPUnits"></param>
		/// <returns name="SimulationResult"></returns>
		static SimulationResult^ ByEnergySimulation(EnergySimulation^ energySimulation, String^ EPReportName, String^ EPReportForString, String^ EPTableName, String^ EPColumnName, String^ EPUnits);

		/*/// <summary>
		/// 
		/// </summary>
		/// <param name="energyModel"></param>
		/// <param name="data"></param>
		/// <param name="alpha"></param>
		/// <returns name="GeometryColor[]"></returns>
		static List<Modifiers::GeometryColor^>^ Display(EnergyModel^ energyModel, Dictionary<String^, Dictionary<String^, Object^>^>^ data, int alpha);*/

		/// <summary>
		/// 
		/// </summary>
		/// <param name="energyModel"></param>
		/// <param name="colors"></param>
		/// <returns name="GeometryColor[]"></returns>
		static List<Modifiers::GeometryColor^>^ Display(EnergyModel^ energyModel, List<DSCore::Color^>^ colors);

		/// <summary>
		/// This outputs the color range of the data. The first element is the colors, the second element is the normalised position within the spectrum. The colors are sorted by the values.
		/// </summary>
		/// <param name="minDomain"></param>
		/// <param name="maxDomain"></param>
		/// <param name="count"></param>
		/// <returns name="ColorRangeData"></returns>
		List<List<int>^>^ LegendRGB(
			[Autodesk::DesignScript::Runtime::DefaultArgument("null")] Nullable<double> minDomain,
			[Autodesk::DesignScript::Runtime::DefaultArgument("null")] Nullable<double> maxDomain,
			[Autodesk::DesignScript::Runtime::DefaultArgument("10")] int count);


		List<double>^ LegendValues(
			[Autodesk::DesignScript::Runtime::DefaultArgument("null")] Nullable<double> minDomain,
			[Autodesk::DesignScript::Runtime::DefaultArgument("null")] Nullable<double> maxDomain,
			[Autodesk::DesignScript::Runtime::DefaultArgument("10")] int count);

		property List<String^>^ Names
		{
			List<String^>^ get();
		}

		property List<double>^ Values
		{
			List<double>^ get();
		}

		property List<double>^ Domain
		{
			List<double>^ get();
		}

		//List<DSCore::Color^>^ ARGB(
		List<List<int>^>^ RGB(
			[Autodesk::DesignScript::Runtime::DefaultArgument("null")] Nullable<double> minDomain, 
			[Autodesk::DesignScript::Runtime::DefaultArgument("null")] Nullable<double> maxDomain);

	public protected:
		/// <summary>
		/// 
		/// </summary>
		SimulationResult(Dictionary<String^, Dictionary<String^, Object^>^>^ data);
		~SimulationResult();

		List<double>^ LegendRatios(
			Nullable<double> minDomain,
			Nullable<double> maxDomain,
			int count,
			double% finalMinDomain,
			double& finalMaxDomain);

	protected:
		Dictionary<String^, Dictionary<String^, Object^>^>^ m_data;
	};
}