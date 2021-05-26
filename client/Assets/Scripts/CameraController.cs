using UnityEngine;

public class CameraController : MonoBehaviour
{
    public Vector2 sensitivity = Vector2.one;
    [Range(-89f, 89f)]
    public float maxX;
    [Range(-89f, 89f)]
    public float minX;

    Vector2 _lastMousePosition;

    void Update()
    {
        if (Input.GetKeyDown(KeyCode.Mouse1))
        {
            _lastMousePosition = Input.mousePosition;
        }
        else if (Input.GetKey(KeyCode.Mouse1))
        {
            var rotation = (Vector2)Input.mousePosition - _lastMousePosition;
            rotation = new Vector2(rotation.y * sensitivity.x, rotation.x * sensitivity.y);
            _lastMousePosition = Input.mousePosition;

            // transform.Rotate(rotation.x, 0f, 0f);
            transform.Rotate(0f, rotation.y, 0f, Space.World);

            var angles = transform.eulerAngles;
            angles.x = Mathf.Clamp(angles.x, minX, maxX);
            transform.eulerAngles = angles;
        }
    }
}
