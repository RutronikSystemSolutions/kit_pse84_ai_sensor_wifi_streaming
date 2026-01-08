using OxyPlot;
using OxyPlot.Axes;
using OxyPlot.Series;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace kit_pse84_ai_wifi_streaming.Views
{
    public partial class AmplitudeTimeView : UserControl
    {
        private const int MAX_POINTS = 300;

        /// <summary>
        /// X Axis
        /// </summary>
        LinearAxis xAxis = new LinearAxis
        {
            MajorGridlineStyle = LineStyle.Dot,
            Position = AxisPosition.Bottom,
            AxislineStyle = LineStyle.Solid,
            AxislineColor = OxyColors.Gray,
            FontSize = 10,
            PositionAtZeroCrossing = true,
            IsPanEnabled = false,
            IsZoomEnabled = true,
            Unit = "Sample index"
        };

        /// <summary>
        /// Y Axis for amplitude
        /// </summary>
        private LinearAxis yAxis = new LinearAxis
        {
            MajorGridlineStyle = LineStyle.Dot,
            AxislineStyle = LineStyle.Solid,
            AxislineColor = OxyColors.Gray,
            FontSize = 10,
            TextColor = OxyColors.Gray,
            Position = AxisPosition.Left,
            IsPanEnabled = false,
            IsZoomEnabled = true,
            Minimum = 0,
            Maximum = 1,
            Unit = "ADC tick",
            Key = "Amp",
        };

        private List<LineSeries> lines = new List<LineSeries>();

        public AmplitudeTimeView()
        {
            InitializeComponent();
            InitPlot();
        }

        private void InitPlot()
        {
            // Raw signals plot
            var timeModel = new PlotModel
            {
                PlotType = PlotType.XY,
                PlotAreaBorderThickness = new OxyThickness(0),
            };

            // Set the axes
            timeModel.Axes.Add(xAxis);
            timeModel.Axes.Add(yAxis);

            // As many line series as antennas
            lines.Clear();
            for (int antennaIndex = 0; antennaIndex < ProjectConfiguration.RADAR_ANTENNA_COUNT; ++antennaIndex)
            {
                LineSeries series = new LineSeries();
                series.Title = string.Format("Antenna {0}", antennaIndex);
                series.YAxisKey = yAxis.Key;
                lines.Add(series);
                timeModel.Series.Add(series);
            }

            plotView.Model = timeModel;
            plotView.InvalidatePlot(true);
        }

        
        public void updateData(ushort frameIndex, double[] amplitudes)
        {
            for (int antennaIndex = 0; antennaIndex < ProjectConfiguration.RADAR_ANTENNA_COUNT; ++antennaIndex)
            {
                lines[antennaIndex].Points.Add(new DataPoint(frameIndex, amplitudes[antennaIndex]));
            }

            // Check for size
            if (lines[0].Points.Count > MAX_POINTS)
            {
                for (int antennaIndex = 0; antennaIndex < ProjectConfiguration.RADAR_ANTENNA_COUNT; ++antennaIndex)
                {
                    lines[antennaIndex].Points.RemoveAt(0);
                }
            }

            plotView.InvalidatePlot(true);
        }
    }
}
