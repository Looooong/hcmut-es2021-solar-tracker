using ChartAndGraph;
using System;
using UnityEngine;

public class SolarPanelStatistics : MonoBehaviour
{
    public GraphChart solarPanelVoltageChart;

    public void AddSystemState(SystemState state)
    {
        var time = DateTimeOffset.FromUnixTimeSeconds(state.timestamp).UtcDateTime;
        solarPanelVoltageChart.DataSource.AddPointToCategoryRealtime("Solar Panel Voltage", time, state.solarPanelVoltage, 1f);
    }
}
