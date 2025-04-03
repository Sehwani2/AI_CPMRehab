import re
import port_set as port  # 포트 연결 헤더
import local_DB_CSV as lo  # 로컬DB(.csv) 헤더
import google_sheet as upload  # 서버DB(구글시트) 헤더

import keyboard  # ESC 키를 감지하기 위한 라이브러리


# 패널 차트에 입력할 프로토콜 데이터를 생성하는 함수
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


# XR21V1410 포트에서 데이터를 읽고 STM32로 전송하는 함수
def read_and_forward_data(ser_xr21, ser_stm32):
    # 차트에 데이터 전송용 변수
    previous_value = 0  # 초기 이전값을 0으로 설정
    current_value = 0  # 초기 현재값을 0으로 설정
    trash = bytes([0x5A, 0xA5, 0x03, 0x82, 0x4F, 0x4B])


    try:
        while True:
            # XR21V1410 포트에서 데이터가 있을 때만 읽기
            if ser_xr21 and ser_xr21.is_open and ser_xr21.in_waiting > 0:
                # XR21V1410 포트에서 가능한 모든 데이터를 읽기
                data = ser_xr21.read(ser_xr21.in_waiting)
                if data:
                    # 데이터를 16진수 형식으로 변환하여 출력
                    formatted_data = " ".join([f"{byte:02X}" for byte in data])
                    print(f"패널: {formatted_data}")

                    if data != trash:

                        # 받은 데이터를 STM32로 전송
                        if ser_stm32 and ser_stm32.is_open:
                            ser_stm32.write(data)
                            print(f"STM32에 전송")
                    else:
                        print(f"쓰레기")

            # STM32 포트에서 데이터가 있을 때만 읽기
            if ser_stm32 and ser_stm32.is_open and ser_stm32.in_waiting > 0:
                line_stm32 = ser_stm32.readline().decode("utf-8").strip()

                # 추후 데이터 띄어쓰기로 분리해서 각 부분별로 저장




                if line_stm32 and line_stm32.isdigit():  # 값이 숫자일 때만 처리
                    print(f"STM32에서 읽음: {line_stm32}")
                    lo.append_to_csv(line_stm32)  # 데이터 CSV에 추가

                    current_value = int(line_stm32)  # 문자열을 정수로 변환

                    # 차트로 데이터 전송
                    protocol_data = create_protocol(current_value, previous_value)
                    print(f"전송할 프로토콜 데이터: {protocol_data.hex().upper()}")
                    ser_xr21.write(protocol_data)  # 패널로 전송
                    print("차트 전송 완료")

                    # 이전값 갱신
                    previous_value = current_value

            # ESC 키 입력이 감지되면 종료
            if keyboard.is_pressed('esc'):
                print("ESC 키가 눌렸습니다. 프로그램을 종료합니다.")
                break  # 반복문 종료하여 프로그램 종료

    except Exception as e:
        print(f"Error in read_and_forward_data: {e}")


# 메인 함수
def main():
    # stm32 포트 열기
    stm32_port = port.find_stm32_port()
    if not stm32_port:
        print("STM32 포트를 찾을 수 없습니다. 프로그램을 종료합니다.")
        return

    xr21v1410_port = port.find_xr21v1410_port()
    if not xr21v1410_port:
        print("DMI패널 포트를 찾을 수 없습니다. 프로그램을 종료합니다.")
        return

    # 시리얼 포트 열기
    ser_stm32 = port.open_serial_port(stm32_port) if stm32_port else None
    ser_xr21 = port.open_serial_port(xr21v1410_port) if xr21v1410_port else None

    if not ser_stm32 or not ser_xr21:
        print("시리얼 포트를 열 수 없습니다. 프로그램을 종료합니다.")
        return

    # CSV 파일 초기화
    lo.initialize_csv()

    # XR21V1410 포트에서 값을 읽고 STM32로 전송하는 함수 호출
    read_and_forward_data(ser_xr21, ser_stm32)

    # 프로그램 종료 시 시리얼 포트 닫기
    if ser_stm32:
        ser_stm32.close()
    if ser_xr21:
        ser_xr21.close()


if __name__ == "__main__":
    main()
