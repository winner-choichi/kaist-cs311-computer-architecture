import subprocess

for i in range(1, 6):
    # runfile 실행
    subprocess.run(["./runfile", f"sample_input/example{i}.s"])
    
    # diff로 비교
    result = subprocess.run(["diff", f"sample_input/example{i}.o", f"sample_output/example{i}.o"])
    # diff 명령의 결과에 따라 처리할 수도 있음