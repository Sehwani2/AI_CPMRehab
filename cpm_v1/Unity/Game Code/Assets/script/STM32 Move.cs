using UnityEngine;
using System.IO.Ports;
using System.Threading;

public class STM32Move : MonoBehaviour
{
    public float moveSpeed = 0.0001f;  // 기본 이동 속도 (값 크기에 비례하여 조정)
    public float maxHeight = 4.5f;    // Y축 최대 범위
    public float minHeight = -4.5f;   // Y축 최소 범위
    private Rigidbody2D rb;

    private SerialPort serialPort;
    private Thread serialThread;
    private float rawPotValue = 0f;  // 시리얼에서 읽은 값

    void Start()
    {
        rb = GetComponent<Rigidbody2D>();

        // 시리얼 포트 설정
        serialPort = new SerialPort("COM13", 115200);
        serialPort.Open();

        // 시리얼 데이터를 읽는 스레드 시작
        serialThread = new Thread(ReadSerialData);
        serialThread.IsBackground = true; // 게임이 종료되면 스레드도 종료됨
        serialThread.Start();
    }

    void Update()
    {
        // 시리얼 값 범위 처리 (값이 0보다 작으면 -값, 값이 0보다 크면 +값)
        float moveInput = rawPotValue * moveSpeed;

        // Y축으로 이동 (rawPotValue가 +일 때 위로, -일 때 아래로)
        rb.linearVelocity = new Vector2(0, moveInput);

        // Y 위치 제한 (화면 밖으로 나가지 않도록 제한)
        float clampedY = Mathf.Clamp(transform.position.y, minHeight, maxHeight);
        transform.position = new Vector3(transform.position.x, clampedY, 0);
    }

    // 시리얼 포트를 통해 STM32에서 값 읽기
    void ReadSerialData()
    {
        while (serialPort.IsOpen)
        {
            try
            {
                string data = serialPort.ReadLine();
                rawPotValue = float.Parse(data);
            }
            catch (System.Exception)
            {
                // 오류 처리 (값을 읽을 수 없으면 기본값 사용)
                rawPotValue = 0f;
            }
        }
    }

    // 게임 종료 시 시리얼 포트 닫기
    void OnApplicationQuit()
    {
        if (serialPort.IsOpen)
        {
            serialPort.Close();
        }

        if (serialThread != null && serialThread.IsAlive)
        {
            serialThread.Abort(); // 스레드 종료
        }
    }
}
