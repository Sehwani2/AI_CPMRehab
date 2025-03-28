import serial
import serial.tools.list_ports
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import re
import keyboard
import pandas as pd
import os
import threading
import csv
import requests
import json

# Google Apps Script 웹 앱 URL
APP_SCRIPT_URL = "https://script.google.com/macros/s/AKfycbyHgejQAJYbrJYC5P2_lNxEdN4IKwDYKvKaOV0Lh8YvQFK4AnOCN4eZXWodEYAc7EZ8/exec"

# STM32 UART 속도 설정
BAUD_RATE = 115200  # STM32와 동일하게 설정

# 시스템에 연결된 모든 시리얼 포트 확인 (STMicroelectronics STLink Virtual COM Port 찾기)
def find_stm32_port():
    ports = list(serial.tools.list_ports.comports())
    for port in ports:
        print(f"포트 이름: {port.device}, 설명: {port.description}")
        # 포트 설명에 'STMicroelectronics STLink Virtual COM Port' 포함 확인
        if "STMicroelectronics STLink Virtual COM Port" in port.description:
            return port.device
    return None

# 자동으로 STM32 포트 찾기
SERIAL_PORT = find_stm32_port()
if SERIAL_PORT:
    print(f"STM32가 연결된 포트: {SERIAL_PORT}")
else:
    raise SystemExit("STM32 포트를 찾을 수 없습니다. 프로그램을 종료합니다.")

# 시리얼 포트 열기
def open_serial_port():
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        if ser.is_open:
            print(f"Serial port {SERIAL_PORT} is open.")
        return ser
    except serial.SerialException as e:
        print(f"Error opening serial port {SERIAL_PORT}: {e}")
        return None

# 시리얼 포트 열기 시도
ser = open_serial_port()
if not ser:
    raise SystemExit("시리얼 포트를 열 수 없습니다. 프로그램을 종료합니다.")

# 그래프 데이터 리스트
adc_values = []
max_points = 50  # 그래프에 표시할 최대 데이터 개수

# CSV 파일 경로 설정
csv_file = "data/patient_data.csv"
last_index = 0  # CSV 파일에서 마지막 인덱스 추적

# CSV 파일 초기화 함수
def initialize_csv():
    if os.path.exists(csv_file):
        os.remove(csv_file)  # 기존 CSV 파일 삭제
    with open(csv_file, mode='w', newline='') as f:
        f.write("key,data\n")  # 새 CSV 파일 작성 및 헤더 추가

# CSV 파일 초기화
initialize_csv()


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


# 그래프 업데이트 함수
def update_graph(frame):
    global adc_values
    try:
        # 그래프 데이터 유지 (최대 max_points 개)
        if len(adc_values) > max_points:
            adc_values.pop(0)

        ax.clear()
        ax.plot(adc_values, marker="o", linestyle="-", color="b", label="ADC Value")
        ax.set_ylim(0, 4095)  # 12비트 ADC 최대값 4095
        ax.set_title("Real-time ADC Data from STM32")
        ax.set_xlabel("Time")
        ax.set_ylabel("ADC Value")
        ax.legend()
    except Exception as e:
        print("Error:", e)

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

# 실시간 데이터를 읽고 저장하는 함수 (스레드)
def read_and_save_data(stop_event):
    global ser
    while not stop_event.is_set():
        try:
            if ser and ser.is_open:
                line = ser.readline().decode("utf-8").strip()  # UART 데이터 읽기
                if line:  # 비어있지 않으면 처리
                    print(f"Received line: {line}")  # 디버깅용: 수신된 데이터 출력
                    match = re.search(r"ADC:\s*(\d+)", line)  # "ADC: 숫자" 포맷 찾기
                    if match:
                        adc_value = int(match.group(1))
                        adc_values.append(adc_value)
                        append_to_csv(adc_value)  # 데이터 CSV에 추가
                        print(f"Received ADC: {adc_value}")  # 디버깅용 출력
        except serial.SerialException:
            print("시리얼 포트가 닫혔습니다. 재연결 시도 중...")
            ser = open_serial_port()
        except Exception as e:
            print("Error:", e)

# 그래프 초기 설정
fig, ax = plt.subplots()

# 수정된 부분: FuncAnimation의 save_count를 명시적으로 설정하거나, cache_frame_data=False로 설정
ani = animation.FuncAnimation(fig, update_graph, interval=10, save_count=100, cache_frame_data=False)  # 10ms마다 업데이트

# 쓰레드 종료를 위한 이벤트 객체
stop_event = threading.Event()

# 데이터 읽기 및 저장 스레드 시작
data_thread = threading.Thread(target=read_and_save_data, args=(stop_event,), daemon=True)
data_thread.start()

# ESC 키를 눌렀을 때 프로그램 종료
while True:
    if keyboard.is_pressed("esc"):  # ESC 키 감지
        plt.close()
        stop_event.set()  # 쓰레드 종료 신호 보내기
        data_thread.join()  # 쓰레드가 종료될 때까지 대기
        print("ESC 키 감지! 프로그램 종료\n")

        read_csv_and_upload(csv_file) # 데이터 구글시트에 업로드

        # 구글 시트에서 최대값과 최소값 가져오기
        print("\n MAX, MIN 값 가져오는 중...")
        max_val, min_val = get_max_min_from_sheet()
        if max_val is not None and min_val is not None:
            print(f"최고값: {max_val}, 최소값: {min_val}")

        break

    # 그래프 실행
    plt.pause(0.01)  # matplotlib 애니메이션을 중지하지 않게 유지

# 프로그램 종료 시 시리얼 닫기
ser.close()
