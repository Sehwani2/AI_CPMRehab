import serial
import time
import re  # 정수 추출을 위한 정규 표현식 사용
import threading  # 쓰레드 모듈

#차트에 데이터 쏴주는 코드


# 프로토콜 데이터를 생성하는 함수
def create_protocol(current_value, previous_value):
    # 프로토콜 데이터 (고정)
    protocol = [0x5A, 0xA5, 0x0D, 0x82, 0x03, 0x10,
                0x5A, 0xA5, 0x01, 0x00, 0x01, 0x02]

    # 값을 16진수로 변환하여 상위 바이트와 하위 바이트로 분리
    current_high_byte = (current_value >> 8) & 0xFF  # 현재값 상위 8비트
    current_low_byte = current_value & 0xFF  # 현재값 하위 8비트

    previous_high_byte = (previous_value >> 8) & 0xFF  # 이전값 상위 8비트
    previous_low_byte = previous_value & 0xFF  # 이전값 하위 8비트

    # 이전값과 현재값을 프로토콜에 추가
    protocol.extend([previous_high_byte, previous_low_byte])  # 이전값
    protocol.extend([current_high_byte, current_low_byte])  # 현재값

    return bytes(protocol)


# COM6에서 값을 읽어오는 함수
def read_from_com6(com6):
    previous_value = 0  # 초기 이전값을 0으로 설정
    current_value = 0  # 초기 현재값을 0으로 설정

    while True:
        try:
            if com6.in_waiting > 0:  # 읽을 데이터가 있는 경우
                data = com6.readline().decode('utf-8').strip()  # 데이터 읽기 및 디코딩
                print(f"COM6에서 읽은 값: {data}")

                # 정수 부분만 추출
                integers = re.findall(r'\d+', data)  # 문자열에서 숫자만 추출
                if integers:  # 추출된 정수가 있는 경우
                    current_value = int(integers[0])  # 첫 번째 정수만 사용
                    print(f"추출된 정수: {current_value}")

                    # 이전값을 현재값으로 갱신
                    # 프로토콜 생성 및 COM4로 전송
                    send_protocol_data(current_value, previous_value)

                    # 이전값 갱신
                    previous_value = current_value

        except Exception as e:
            print(f"COM6 읽기 오류: {e}")
            break


# COM4로 프로토콜 데이터를 전송하는 함수
def send_protocol_data(current_value, previous_value):
    try:
        protocol_data = create_protocol(current_value, previous_value)
        print(f"전송할 프로토콜 데이터: {protocol_data.hex().upper()}")
        com_port.write(protocol_data)  # COM4로 전송
        print("프로토콜 데이터를 COM4로 전송 완료")
    except Exception as e:
        print(f"전송 오류: {e}")


# 메인 코드
try:
    # COM6 포트 설정 (값 읽기용)
    com6 = serial.Serial('COM6', baudrate=115200, timeout=1)
    print("COM6 연결 성공")

    # COM4 포트 설정 (값 전송용)
    com_port = serial.Serial('COM4', baudrate=115200, timeout=1)
    print("COM4 연결 성공")

    # COM6 읽기용 쓰레드 시작
    reader_thread = threading.Thread(target=read_from_com6, args=(com6,))
    reader_thread.start()

    # 쓰레드가 종료될 때까지 대기
    reader_thread.join()

except serial.SerialException as e:
    print(f"시리얼 연결 오류: {e}")

except Exception as e:
    print(f"오류 발생: {e}")

finally:
    # 포트 닫기
    if 'com6' in locals() and com6.is_open:
        com6.close()
    if 'com_port' in locals() and com_port.is_open:
        com_port.close()
    print("시리얼 포트 닫힘")
