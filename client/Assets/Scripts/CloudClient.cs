using BestHTTP.WebSocket;
using UnityEngine;

public class CloudClient : MonoBehaviour
{
    public SolarPanelController solarPanelController;
    public SolarPanelStatistics solarPanelStatistics;

    WebSocket _ws;

    void Awake()
    {
        _ws = new WebSocket(new System.Uri("ws://localhost:8080/ws"));

        _ws.OnOpen += _ =>
        {
            Debug.Log("Connected to WebSocket server.");
            _ws.Send("{\"event\":\"FAKE_DATA\"}"); // TODO: Remove this
        };

        _ws.OnClosed += (_, code, message) =>
        {
            Debug.Log($"Disconnected from WebSocet server with code {code}, message: {message}.");
        };

        _ws.OnMessage += (_, message) =>
        {
            Debug.Log($"Received message from WebSocket server:\n{message}");

            var eventWrapper = JsonUtility.FromJson<EventWrapper>(message);

            if (System.Enum.TryParse(eventWrapper.@event, out Event @event))
            {
                switch (@event)
                {
                    case Event.UPDATE_CONFIG:
                        var config = JsonUtility.FromJson<ControlConfigWrapper>(message).payload;
                        solarPanelController.UpdateConfig(config);
                        break;

                    case Event.UPDATE_STATE:
                        var state = JsonUtility.FromJson<SystemStateWrapper>(message).payload;
                        solarPanelStatistics.AddSystemState(state);
                        break;
                }
            }
            else
            {
                Debug.LogWarning($"Cannot parse event \"{eventWrapper.@event}\", ignored,");
            }
        };

        _ws.Open();
    }

    void OnDestroy()
    {
        if (_ws.IsOpen) _ws.Close();
    }

    enum Event
    {
        UPDATE_CONFIG,
        UPDATE_STATE,
    }

    struct EventWrapper
    {
        public string @event;
    }

    struct ControlConfigWrapper
    {
        public ControlConfig payload;
    }

    struct SystemStateWrapper
    {
        public SystemState payload;
    }
}
