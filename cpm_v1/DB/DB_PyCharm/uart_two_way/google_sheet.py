import csv
import requests
import json


# Google Apps Script 웹 앱 URL
APP_SCRIPT_URL = "https://script.google.com/macros/s/AKfycbyHgejQAJYbrJYC5P2_lNxEdN4IKwDYKvKaOV0Lh8YvQFK4AnOCN4eZXWodEYAc7EZ8/exec"



def read_csv_and_upload(file_path):
    # .csv 파일 읽기
    print("입력된 데이터 전송중...\n")
    with open(file_path, 'r', encoding='utf-8') as file:
        csv_reader = csv.reader(file)
        data = [row for row in csv_reader]  # 모든 데이터를 리스트로 저장

    # JSON 형식으로 변환
    json_data = json.dumps(data)

    # Google Apps Script로 POST 요청
    try:
        response = requests.post(APP_SCRIPT_URL, data=json_data)
        print("구글 시트 업로드 결과:", response.text)
    except Exception as e:
        print(f"에러 발생: {e}")


# 최고값과 최소값을 구하는 함수 (구글 시트에서 가져오기)
def get_max_min_from_sheet():

    print("\n MAX, MIN 값 가져오는 중...")

    try:
        response = requests.get(APP_SCRIPT_URL)  # Google Apps Script의 doGet() 호출
        if response.status_code == 200: # 200이란? : 서버에 응답이 성공적으로 들어왔을 때
            data = response.json()
            return data['max'], data['min']
        else:
            print("Error: 구글 시트에서 데이터를 가져오는 데 실패했습니다.")
            return None, None
    except Exception as e:
        print(f"에러 발생: {e}")
        return None, None
