import os


# CSV 파일 경로 설정
csv_file = "data/patient_data.csv"
last_index = 0  # CSV 파일에서 마지막 인덱스 추적

# CSV 파일 초기화 함수
def initialize_csv():
    if os.path.exists(csv_file):
        os.remove(csv_file)  # 기존 CSV 파일 삭제
    with open(csv_file, mode='w', newline='') as f:
        f.write("key,data\n")  # 새 CSV 파일 작성 및 헤더 추가



# CSV 파일에 데이터 추가 함수 (매번 읽지 않고 추가만)
def append_to_csv(adc_value):
    global last_index
    try:
        last_index += 1
        # 파일 열기 모드 'a' (append 모드)
        with open(csv_file, mode='a', newline='') as f:
            # 새로운 데이터를 추가 (번호, 데이터)
            new_data = f"{last_index},{adc_value}\n"
            f.write(new_data)  # 파일에 추가
    except Exception as e:
        print(f"Error appending data to CSV: {e}")
