﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Grasshopper.Kernel;
using Grasshopper.Kernel.Data;
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
            pManager.AddGenericParameter("Geometry", "Geometry", "Geometry", GH_ParamAccess.tree);
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

            //Object geometry = ToGeometry(topology);

            //List<Object> output = new List<Object>();
            Grasshopper.DataTree<Object> output = new Grasshopper.DataTree<Object>();
            List<int> path = new List<int>();
            path.Add(0);
            RecursiveGeometry(topology, path, ref output);

            //Grasshopper.DataTree<Object> tree = new Grasshopper.DataTree<Object>();
            //int i = 0;
            //foreach (Object o in output)
            //{
            //    // a list
            //    ICollection<Object> innerList = o as ICollection<Object>;
            //    if (innerList != null)
            //    {
            //        tree.AddRange(innerList, new GH_Path(new int[] { 0, i }));
            //    }else
            //    {
            //        tree.Add(o, new GH_Path(i));
            //    }
            //    i++;
            //}
            
            DA.SetDataTree(0, output);
        }

        GH_Path ListToTreePath(List<int> path)
        {
            if(path.Count == 0)
            {
                return null;
            }

            if(path.Count == 1)
            {
                return new GH_Path(path[0]);
            }

            return new GH_Path(path.ToArray());
        }

        void AddObjectToTree(Object o, List<int> path, ref Grasshopper.DataTree<Object> output)
        {
            ICollection<Object> innerList = o as ICollection<Object>;
            if (innerList != null)
            {
                List<int> newPath = new List<int>(path);
                newPath.Add(0);
                int i = 0;
                foreach (Object innerO in innerList)
                {
                    List<int> newPath2 = new List<int>(path);
                    newPath2.Add(i);
                    AddObjectToTree(innerO, newPath2, ref output);
                    ++i;
                }
            }
            else
            {
                output.Add(o, ListToTreePath(path));
            }
        }

        void RecursiveGeometry(Topology topology, List<int> path, ref Grasshopper.DataTree<Object> output)
        {
            Object geometry = ToGeometry(topology);
            AddObjectToTree(geometry, path, ref output);

            List<Topology> subContents = topology.SubContents;
            int i = 0;
            foreach(Topology subContent in subContents)
            {
                List<int> newPath = new List<int>(path);
                newPath.Add(1);
                newPath.Add(i);
                RecursiveGeometry(subContent, newPath, ref output);
                ++i;
            }
        }

        private object ToGeometry(Topology topology)
        {
            if(topology == null)
            {
                return null;
            }

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
            
            Wire wire = topology as Wire;
            if (wire != null)
            {
                return ToPolyCurve(wire);
            }

            Face face = topology as Face;
            if (face != null)
            {
                return ToSurface(face);
            }

            Shell shell = topology as Shell;
            if (shell != null)
            {
                return ToBrep(shell);
            }

            Cell cell = topology as Cell;
            if (cell != null)
            {
                return ToBrep(cell);
            }

            CellComplex cellComplex = topology as CellComplex;
            if (cellComplex != null)
            {
                return ToList(cellComplex);
            }

            Cluster cluster = topology as Cluster;
            if (cluster != null)
            {
                return ToList(cluster);
            }

            Aperture aperture = topology as Aperture;
            if (aperture != null)
            {
                return ToGeometry(aperture.Topology);
            }

            throw new Exception("The type of the input topology is not recognized.");
        }

        private object ToList(Topology topology)
        {
            List<Topology> subTopologies = topology.SubTopologies;
            List<Object> ghGeometries = new List<Object>();
            foreach(Topology subTopology in subTopologies)
            {
                Object ghGeometry = ToGeometry(subTopology);
                ghGeometries.Add(ghGeometry);
            }
            return ghGeometries;
        }

        private Brep ToBrep(Topology topology)
        {
            List<Face> faces = topology.Faces;
            List<Brep> ghBrepSurfaces = new List<Brep>();
            foreach(Face face in faces)
            {
                Brep ghBrepSurface = ToSurface(face);
                ghBrepSurfaces.Add(ghBrepSurface);
            }
            
            if(ghBrepSurfaces.Count == 0)
            {
                return null;
            }else if(ghBrepSurfaces.Count == 1)
            {
                return ghBrepSurfaces[0];
            }else
            {
                Brep ghBrep = ghBrepSurfaces[0];
                for (int i = 1; i < ghBrepSurfaces.Count; ++i)
                {
                    ghBrep.Join(ghBrepSurfaces[i], 0.0001, true);
                }
                return ghBrep;
            }
        }

        private void ProcessFace(
            Wire wire, ref Brep ghBrep, BrepFace ghBrepFace, BrepLoopType ghBrepLoopType, Rhino.Geometry.NurbsSurface ghNurbsSurface, 
            Dictionary<Edge, Tuple<int, int>> edge2DIndices, Dictionary<Edge, BrepEdge> edgeIndices)
        {
            List<Edge> edges = wire.Edges;
            BrepLoop ghBrepLoop = ghBrep.Loops.Add(ghBrepLoopType, ghBrepFace);

            // 2f.For each loop, add a trim(2D edge)
            List<BrepEdge> ghEdges = new List<BrepEdge>();
            List<Tuple<Curve, int, Curve, int>> gh2DCurves = new List<Tuple<Curve, int, Curve, int>>(); // original curve, index, reverse curve, reverse index
            foreach (Edge edge in edges)
            {
                Tuple<int, int> thisEdge2DIndices = edge2DIndices.
                    Where(edgeIndexPair => edgeIndexPair.Key.IsSame(edge)).
                    Select(edgeIndexPair => edgeIndexPair.Value).
                    FirstOrDefault();

                int thisEdge2DIndex = thisEdge2DIndices.Item1;
                int thisReverseEdge2DIndex = thisEdge2DIndices.Item2;

                Curve ghCurve2D = ghBrep.Curves2D[thisEdge2DIndex];
                Curve ghReverseCurve2D = ghBrep.Curves2D[thisReverseEdge2DIndex];
                gh2DCurves.Add(Tuple.Create(ghCurve2D, thisEdge2DIndex, ghReverseCurve2D, thisReverseEdge2DIndex));

                BrepEdge ghBrepEdge = edgeIndices.
                    Where(edgeIndexPair => edgeIndexPair.Key.IsSame(edge)).
                    Select(edgeIndexPair => edgeIndexPair.Value).
                    FirstOrDefault();

                String ghBrepEdgeLog = "";
                if (!ghBrepEdge.IsValidWithLog(out ghBrepEdgeLog))
                {
                    throw new Exception("Fails to create a valid Brep with the following message: " + ghBrepEdgeLog);
                }

                ghEdges.Add(ghBrepEdge);
            }

            for (int currentEntryID = 0; currentEntryID < gh2DCurves.Count; ++currentEntryID)
            {
                int previousEntryID = currentEntryID - 1;
                if (previousEntryID < 0)
                {
                    previousEntryID = edges.Count - 1;
                }

                bool isCurrentStartEqualToPreviousStart = gh2DCurves[currentEntryID].Item1.PointAtStart.DistanceTo(
                                                            gh2DCurves[previousEntryID].Item1.PointAtStart) < 0.0001;
                bool isCurrentStartEqualToPreviousEnd = gh2DCurves[currentEntryID].Item1.PointAtStart.DistanceTo(
                                                            gh2DCurves[previousEntryID].Item1.PointAtEnd) < 0.0001;
                bool isTrimReversed = false;
                if (!isCurrentStartEqualToPreviousStart && !isCurrentStartEqualToPreviousEnd)
                {
                    // Reverse trim
                    isTrimReversed = true;
                }

                BrepTrim ghBrepTrim = ghBrep.Trims.Add(
                    ghEdges[currentEntryID],       // 3D edge
                    isTrimReversed,                // is reversed?
                    ghBrepLoop,                    // 2D loop
                    isTrimReversed ? gh2DCurves[currentEntryID].Item4 : gh2DCurves[currentEntryID].Item2);  // 2D curve index, use the reversed one if reversed

                ghBrepTrim.IsoStatus = ghNurbsSurface.IsIsoparametric(gh2DCurves[currentEntryID].Item1);
                ghBrepTrim.TrimType = BrepTrimType.Boundary;
                ghBrepTrim.SetTolerances(0.0, 0.0);

                String ghBrepTrimLog = "";
                if (!ghBrepTrim.IsValidWithLog(out ghBrepTrimLog))
                {
                    throw new Exception("Fails to create a valid BrepTrim with the following message: " + ghBrepTrimLog);
                }
            }

            String brepLoopLog = "";
            if (!ghBrepLoop.IsValidWithLog(out brepLoopLog))
            {
                throw new Exception("Fails to create a valid outer BrepLoop with the following message: " + brepLoopLog);
            }
        }

        private Brep ToSurface(Face face)
        {
            // 1. Compute the base NURBS surface
            // Based on https://developer.rhino3d.com/api/RhinoCommon/html/M_Rhino_Geometry_NurbsSurface_Create.htm
            Topologic.NurbsSurface nurbsSurface = face.BasicGeometry as Topologic.NurbsSurface;
            if(nurbsSurface == null)
            {
                throw new Exception("An invalid surface is created.");
            }

            int uDegree = nurbsSurface.UDegree;
            int vDegree = nurbsSurface.VDegree;
            bool isRational = nurbsSurface.IsURational && nurbsSurface.IsVRational;
            int uCount = nurbsSurface.NumOfUControlVertices;
            int vCount = nurbsSurface.NumOfVControlVertices;

            Rhino.Geometry.NurbsSurface ghNurbsSurface = Rhino.Geometry.NurbsSurface.Create(
                3, 
                isRational,
                uDegree + 1,
                vDegree + 1,
                uCount,
                vCount
                );

            int i = 0;
            for (int u = 0; u < uCount; ++u)
            {
                for (int v = 0; v < vCount; ++v)
                {
                    Topologic.Vertex controlVertex = nurbsSurface.ControlVertex(u, v);
                    ghNurbsSurface.Points.SetControlPoint(u, v, ToPoint(controlVertex));
                    ++i;
                }
            }

            List<double> uKnots = nurbsSurface.UKnots;
            uKnots = uKnots.GetRange(1, uKnots.Count - 2);
            for (int u = 0; u < uKnots.Count; u++)
            {
                ghNurbsSurface.KnotsU[u] = uKnots[u];
            }

            List<double> vKnots = nurbsSurface.VKnots;
            vKnots = vKnots.GetRange(1, vKnots.Count - 2);
            for (int v = 0; v < vKnots.Count; v++)
            {
                ghNurbsSurface.KnotsV[v] = vKnots[v];
            }

            if (!ghNurbsSurface.IsValid)
            {
                throw new Exception("A valid surface cannot be created from this Face.");
            }

            // 2. Create the Brep
            //Brep ghBrep = ghNurbsSurface.ToBrep();
            Brep ghBrep = new Brep();

            // 2a. Add vertices
            List<Vertex> vertices = face.Vertices;
            foreach (Vertex vertex in vertices)
            {
                Point3d ghPoint = ToPoint(vertex);
                BrepVertex ghBrepVertex = ghBrep.Vertices.Add(ghPoint, 0.0);

                String ghBrepVertexLog = "";
                if (!ghBrepVertex.IsValidWithLog(out ghBrepVertexLog))
                {
                    throw new Exception("Fails to create a valid BrepVertex with the following message: " + ghBrepVertexLog);
                }
            }

            // 2b. Add 3D curves and edges. The index dictionaries are used to identify the IDs of the edges.
            List<Edge> edges = face.Edges;
            Dictionary<Edge, Tuple<int, int>> edge2DIndices = new Dictionary<Edge, Tuple<int, int>>(); // edge, curve, reverse curve
            Dictionary<Edge, int> edge3DIndices = new Dictionary<Edge, int>();
            Dictionary<Edge, BrepEdge> edgeIndices = new Dictionary<Edge, BrepEdge>();
            foreach (Edge edge in edges)
            {
                Curve ghCurve3D = ToCurve(edge);
                Curve ghCurve2D = ghNurbsSurface.Pullback(ghCurve3D, 0.0001);
                int curve3DID = ghBrep.Curves3D.Add(ghCurve3D);
                int curve2DID = ghBrep.Curves2D.Add(ghCurve2D);

                Curve ghReverseCurve2D = ghCurve2D.DuplicateCurve();
                ghReverseCurve2D.Reverse();
                int reverseCurve2DID = ghBrep.Curves2D.Add(ghReverseCurve2D);

                Point3d ghStartPoint = ghCurve3D.PointAtStart;
                Point3d ghEndPoint = ghCurve3D.PointAtEnd;

                int startVertexIndex = -1;
                int endVertexIndex = -1;
                foreach (BrepVertex ghBrepVertex in ghBrep.Vertices)
                {
                    Point3d ghBrepPoint = ghBrepVertex.Location;

                    if (startVertexIndex == -1 && ghBrepPoint.DistanceTo(ghStartPoint) < 0.0001)
                    {
                        startVertexIndex = ghBrepVertex.VertexIndex;
                    }

                    if (endVertexIndex == -1 && ghBrepPoint.DistanceTo(ghEndPoint) < 0.0001)
                    {
                        endVertexIndex = ghBrepVertex.VertexIndex;
                    }
                }

                BrepEdge ghBrepEdge = ghBrep.Edges.Add(startVertexIndex, endVertexIndex, curve3DID, 0.0001);

                String brepEdgeLog = "";
                if (!ghBrepEdge.IsValidWithLog(out brepEdgeLog))
                {
                    throw new Exception("Fails to create a valid BrepEdge with the following message: " + brepEdgeLog);
                }

                edge3DIndices.Add(edge, curve3DID);
                edge2DIndices.Add(edge, Tuple.Create(curve2DID, reverseCurve2DID));
                edgeIndices.Add(edge, ghBrepEdge);
            }

            // 2c. Add surface
            int ghSurfaceIndex = ghBrep.AddSurface(ghNurbsSurface);

            // 2d. Add face
            BrepFace ghBrepFace = ghBrep.Faces.Add(ghSurfaceIndex);

            // 2e.Create outer loop
            Wire outerWire = face.ExternalBoundary;
            ProcessFace(outerWire, ref ghBrep, ghBrepFace, BrepLoopType.Outer, ghNurbsSurface, edge2DIndices, edgeIndices);

            // 2g. Create inner loops
            List<Wire> innerWires = face.InternalBoundaries;
            foreach (Wire innerWire in innerWires)
            {
                ProcessFace(innerWire, ref ghBrep, ghBrepFace, BrepLoopType.Inner, ghNurbsSurface, edge2DIndices, edgeIndices);
            }

            String brepFaceLog = "";
            if (!ghBrepFace.IsValidWithLog(out brepFaceLog))
            {
                throw new Exception("Fails to create a valid Face with the following message: " + brepFaceLog);
            }

            //ghBrep.Compact();

            String brepLog = "";
            if (!ghBrep.IsValidWithLog(out brepLog))
            {
                throw new Exception("Fails to create a valid Brep with the following message: " + brepLog);
            }

            if (!ghBrep.IsValidGeometry(out brepLog))
            {
                throw new Exception("Fails to create a valid Brep with the following message: " + brepLog);
            }

            if (!ghBrep.IsValidTopology(out brepLog))
            {
                throw new Exception("Fails to create a valid Brep with the following message: " + brepLog);
            }

            if (!ghBrep.IsValidTolerancesAndFlags(out brepLog))
            {
                throw new Exception("Fails to create a valid Brep with the following message: " + brepLog);
            }


            return ghBrep;
        }

        private PolyCurve ToPolyCurve(Wire wire)
        {
            PolyCurve ghPolyCurve = new PolyCurve();
            List<Edge> edges = wire.Edges;
            foreach (Edge edge in edges)
            {
                Curve ghCurve = ToCurve(edge);

                try
                {
                    ghPolyCurve.Append(ghCurve);
                }
                catch
                {
                    throw new Exception("Fails creating a PolyCurve from a Wire.");
                }
            }

            String log = "";
            if(!ghPolyCurve.IsValidWithLog(out log))
            {
                throw new Exception(log);
            }

            return ghPolyCurve;
        }

        private Curve ToCurve(Edge edge)
        {
            Object edgeGeometry = edge.BasicGeometry;
            Curve ghCurve = null;
            Topologic.Line line = edgeGeometry as Topologic.Line;
            if(line != null)
            {
                ghCurve = ToLine(edge);
            }

            Topologic.NurbsCurve nurbsCurve = edgeGeometry as Topologic.NurbsCurve;
            if (nurbsCurve != null)
            {
                ghCurve = ToNurbsCurve(nurbsCurve);
            }

            if (ghCurve == null)
            {
                //throw new Exception("This Edge creates an unrecognized Geometry.");
                return null;
            }

            bool isReversed = edge.IsReversed;

            // IF ENABLED, WILL MAKE THE FACE VALID, BUT CELL INVALID
            //if (isReversed)
            //{
            //    bool successful = ghCurve.Reverse();
            //}

            return ghCurve;
        }

        private Rhino.Geometry.NurbsCurve ToNurbsCurve(Topologic.NurbsCurve nurbsCurve)
        {
            // Based on https://developer.rhino3d.com/api/RhinoCommon/html/P_Rhino_Geometry_NurbsCurve_Knots.htm
            bool isPeriodic = nurbsCurve.IsPeriodic;
            bool isRational = nurbsCurve.IsRational;
            int degree = nurbsCurve.Degree;
            List<Topologic.Vertex> controlVertices = nurbsCurve.ControlVertices;
            List<Point3d> ghControlPoints = new List<Point3d>();

            Rhino.Geometry.NurbsCurve ghNurbsCurve = new Rhino.Geometry.NurbsCurve(3, isRational, degree + 1, controlVertices.Count);
            
            int i = 0;
            foreach(Topologic.Vertex controlVertex in controlVertices)
            {
                Point3d ghControlPoint = ToPoint(controlVertex);
                ghControlPoints.Add(ghControlPoint);
                ghNurbsCurve.Points.SetPoint(i, ghControlPoint);
                ++i;
            }

            List<double> knots = nurbsCurve.Knots;
            knots = knots.GetRange(1, knots.Count - 2);
            i = 0;
            foreach (double knot in knots)
            {
                ghNurbsCurve.Knots[i] = knot;
                ++i;
            }

            String log = "";
            if(ghNurbsCurve.IsValidWithLog(out log))
            {
                return ghNurbsCurve;
            }

            throw new Exception("A valid curve cannot be created from this Edge.");
        }

        private LineCurve ToLine(Edge edge)
        {
            Vertex startVertex = edge.StartVertex;
            Point3d ghStartPoint = ToPoint(startVertex);
            Vertex endVertex = edge.EndVertex;
            Point3d ghEndPoint = ToPoint(endVertex);

            return new LineCurve(ghStartPoint, ghEndPoint);
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