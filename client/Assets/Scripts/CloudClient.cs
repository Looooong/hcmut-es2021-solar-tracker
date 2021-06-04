using BestHTTP.WebSocket;
using Newtonsoft.Json;
using Newtonsoft.Json.Converters;
using Newtonsoft.Json.Linq;
using UnityEngine;

public class CloudClient : MonoBehaviour
{
#if UNITY_EDITOR
    const string WebSocketURL = "ws://localhost:8080/ws";
#else
    const string WebSocketURL = "wss://hcmut-es2021-solar-tracker.herokuapp.com/ws";
#endif
    public SolarPanelController solarPanelController;

    WebSocket _ws;

    public void UpdateConfig(ControlConfig config)
    {
        if (!_ws.IsOpen)
        {
            Debug.LogWarning("WebSocket connection is not established.");
            return;
        }

        var payload = new ControlConfigWrapper
        {
            @event = Event.UPDATE_CONFIG,
            payload = config,
        };

        _ws.Send(JsonConvert.SerializeObject(payload));
    }

    void Awake()
    {
        _ws = new WebSocket(new System.Uri(WebSocketURL));

        _ws.OnOpen += _ =>
        {
            Debug.Log("Connected to WebSocket server.");
        };

        _ws.OnClosed += (_, code, message) =>
        {
            Debug.Log($"Disconnected from WebSocet server with code {code}, message: {message}.");
        };

        _ws.OnMessage += (_, message) =>
        {
            var root = JToken.Parse(message);
            var @event = root["event"];

            if (@event != null)
            {
                switch (@event.ToObject<Event>())
                {
                    case Event.UPDATE_CONFIG:
                        solarPanelController.UpdateControlConfig(root["payload"].ToObject<ControlConfig>());
                        break;

                    case Event.UPDATE_STATE:
                        solarPanelController.AddSystemState(root["payload"].ToObject<SystemState>());
                        break;
                }
            }
            else
            {
                Debug.LogWarning("Cannot parse WebSocket message, ignored,");
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
        [JsonConverter(typeof(StringEnumConverter))]
        public Event @event;
        public ControlConfig payload;
    }

    struct SystemStateWrapper
    {
        [JsonConverter(typeof(StringEnumConverter))]
        public Event @event;
        public SystemState payload;
    }
}
