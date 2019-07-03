﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Grasshopper.Kernel;
using Rhino.Geometry;
using Topologic;

namespace TopologicGH
{
    public class TopologyGeometry : GH_Component
    {
        public TopologyGeometry()
          : base("Topology.Geometry", "Topology.Geometry", "Creates a geometry from the Topology.", "Topologic", "Topology")
        {
        }

        /// <summary>
        /// Registers all the input parameters for this component.
        /// </summary>
        protected override void RegisterInputParams(GH_Component.GH_InputParamManager pManager)
        {
            pManager.AddGenericParameter("Topology", "Topology", "Topology", GH_ParamAccess.item);
        }

        /// <summary>
        /// Registers all the output parameters for this component.
        /// </summary>
        protected override void RegisterOutputParams(GH_Component.GH_OutputParamManager pManager)
        {
            pManager.AddGenericParameter("Geometry", "Geometry", "Geometry", GH_ParamAccess.item);
        }

        /// <summary>
        /// This is the method that actually does the work.
        /// </summary>
        /// <param name="DA">The DA object is used to retrieve from inputs and store in outputs.</param>
        protected override void SolveInstance(IGH_DataAccess DA)
        {
            // Declare a variable for the input String
            Topologic.Topology topology = null;

            // Use the DA object to retrieve the data inside the first input parameter.
            // If the retieval fails (for example if there is no data) we need to abort.
            if (!DA.GetData(0, ref topology)) { return; }

            // If the retrieved data is Nothing, we need to abort.
            // We're also going to abort on a zero-length String.
            if (topology == null) { return; }
            //if (data.Length == 0) { return; }

            // Convert the String to a character array.
            //char[] chars = data.ToCharArray();

            // Reverse the array of character.
            Object geometry = ToGeometry(topology);

            // Use the DA object to assign a new String to the first output parameter.
            DA.SetData(0, geometry);
        }

        private object ToGeometry(Topology topology)
        {
            Vertex vertex = topology as Vertex;
            if (vertex != null)
            {
                return ToPoint(vertex);
            }

            Edge edge = topology as Edge;
            if (edge != null)
            {
                return ToCurve(edge);
            }


            throw new Exception("The type of the input topology is not recognized.");
        }

        private object ToCurve(Edge edge)
        {
            Object edgeGeometry = edge.BasicGeometry;

            Topologic.Line line = edgeGeometry as Topologic.Line;
            if(line != null)
            {
                return ToLine(edge);
            }

            Topologic.NurbsCurve nurbsCurve = edgeGeometry as Topologic.NurbsCurve;
            if (nurbsCurve != null)
            {
                return ToNurbsCurve(nurbsCurve);
            }

            throw new Exception("This Edge creates an unrecognized Geometry.");
        }

        private object ToNurbsCurve(Topologic.NurbsCurve nurbsCurve)
        {
            bool isPeriodic = nurbsCurve.IsPeriodic;
            int degree = nurbsCurve.Degree;
            List<Topologic.Vertex> controlVertices = nurbsCurve.ControlVertices;
            List<Point3d> ghControlPoints = new List<Point3d>();
            foreach(Topologic.Vertex controlVertex in controlVertices)
            {
                Point3d ghControlPoint = ToPoint(controlVertex);
                ghControlPoints.Add(ghControlPoint);
            }
            return Rhino.Geometry.NurbsCurve.Create(isPeriodic, degree, ghControlPoints);
        }

        private object ToLine(Edge edge)
        {
            Vertex startVertex = edge.StartVertex;
            Point3d ghStartPoint = ToPoint(startVertex);
            Vertex endVertex = edge.EndVertex;
            Point3d ghEndPoint = ToPoint(endVertex);
            return new Rhino.Geometry.LineCurve(ghStartPoint, ghEndPoint);
        }

        private Point3d ToPoint(Vertex vertex)
        {
            List<double> coordinates = vertex.Coordinates;
            return new Point3d(coordinates[0], coordinates[1], coordinates[2]);
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
            get { return new Guid("25a0bb53-da53-4789-9e43-a57060481a53"); }
        }
    }
}
