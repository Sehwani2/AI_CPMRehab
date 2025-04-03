using TMPro;
using UnityEngine;
using UnityEngine.SceneManagement;  // 씬 관리를 위한 네임스페이스 추가

public class ObstacleSpawner : MonoBehaviour
{
    public GameObject wallPrefab;  // 벽 프리팹 (WallPrefab)
    public float spawnRate = 5f;   // 벽 생성 간격 (초 단위)
    public float minY = -3f;       // 벽의 최소 Y 위치
    public float maxY = 3f;        // 벽의 최대 Y 위치
    public float gap = 0.5f;       // Y 위치 간격 (0.5 간격으로 생성)

    public TMP_Text tmpText;  // TMP 텍스트를 표시할 UI 텍스트
    private int score = 0;  // 점수 변수

    void Start()
    {
        InvokeRepeating("SpawnWall", 1f, spawnRate);  // 일정 시간마다 벽 생성
        UpdateScoreUI(); // 점수 UI 초기화
    }

    void Update()
    {
        if (Input.GetKeyDown(KeyCode.Escape)) // ESC 키 감지
        {
            Application.Quit(); // 게임 종료
        }

        if (Input.GetKeyDown(KeyCode.R)) // R 키 감지
        {
            SceneManager.LoadScene(SceneManager.GetActiveScene().name); // 현재 씬 다시 로드
        }
    }

    void SpawnWall()
    {
        float holePosition = Mathf.Round(Random.Range(minY, maxY) / gap) * gap;
        GameObject wall = Instantiate(wallPrefab, new Vector3(10f, holePosition, 0), Quaternion.identity);
    }

    public void IncreaseScore()
    {
        score++;
        UpdateScoreUI();
    }

    public void minusScore()
    {
        score--;
        if (score <= 0) score = 0;
        UpdateScoreUI();
    }

    void UpdateScoreUI()
    {
        if (tmpText != null)
        {
            tmpText.text = "" + score;
        }
    }
}
