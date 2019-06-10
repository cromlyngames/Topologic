﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Grasshopper.Kernel;
using Rhino.Geometry;

namespace TopologicGrasshopper
{
    public class TopologyByImportedBRep : GH_Component
    {

        public TopologyByImportedBRep()
          : base("Topology.ByImportedBRep", "Topology.ByImportedBRep", "Imports a Topology from a BRep file (.brep).", "Topologic", "Topology")
        {
        }

        /// <summary>
        /// Registers all the input parameters for this component.
        /// </summary>
        protected override void RegisterInputParams(GH_Component.GH_InputParamManager pManager)
        {
            pManager.AddTextParameter("Path", "Path", "Path", GH_ParamAccess.item);
        }

        /// <summary>
        /// Registers all the output parameters for this component.
        /// </summary>
        protected override void RegisterOutputParams(GH_Component.GH_OutputParamManager pManager)
        {
            pManager.AddGenericParameter("Topology", "Topology", "Topology", GH_ParamAccess.item);
        }

        /// <summary>
        /// This is the method that actually does the work.
        /// </summary>
        /// <param name="DA">The DA object is used to retrieve from inputs and store in outputs.</param>
        protected override void SolveInstance(IGH_DataAccess DA)
        {
            // Declare a variable for the input String
            String path = null;

            //// Use the DA object to retrieve the data inside the first input parameter.
            //// If the retieval fails (for example if there is no data) we need to abort.
            if (!DA.GetData(0, ref path)) { return; }

            //// If the retrieved data is Nothing, we need to abort.
            //// We're also going to abort on a zero-length String.
            if (path == null) { return; }
            //if (data.Length == 0) { return; }

            // Convert the String to a character array.
            //char[] chars = data.ToCharArray();

            // Reverse the array of character.
            Topologic.Topology topology = Topologic.Topology.ByImportedBRep(path);

            // Use the DA object to assign a new String to the first output parameter.
            DA.SetData(0, topology);
        }

        /// <summary>
        /// Provides an Icon for the component.
        /// </summary>
        protected override System.Drawing.Bitmap Icon
        {
            get
            {
                //You can add image files to your project resources and access them like this:
                // return Resources.IconForThisComponent;
                return Resources.NMT_borderless_logo_small;
            }
        }

        /// <summary>
        /// Gets the unique ID for this component. Do not change this ID after release.
        /// </summary>
        public override Guid ComponentGuid
        {
            get { return new Guid("8425e907-b01d-4ea9-9351-687cdb8a8063"); }
        }
    }
}
