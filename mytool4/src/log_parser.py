# 使用方法：python3 log_parser.py  会生成一个result.xlsx

import os
import re
import pandas as pd

def parse_filename(filename):
    """从文件名中提取参数（e, mu, cs）"""
    pattern = r'_e(\d+)_mu(\d+)_cs(\d+)'
    match = re.search(pattern, filename)
    if match:
        e_threshold = f"1e-{match.group(1)}"  # 转换为科学计数法字符串
        mutation_rate = int(match.group(2)) / 100  # 0.01 → 0.01
        crossover_rate = int(match.group(3)) / 100
        return e_threshold, mutation_rate, crossover_rate
    return None, None, None

def get_last_two_lines(filepath):
    """高效获取文件最后两行（避免加载整个文件）"""
    last_line, second_last_line = "", ""
    with open(filepath, 'r') as f:
        for line in f:
            line = line.strip()
            if line:  # 跳过空行
                second_last_line = last_line
                last_line = line
    return second_last_line, last_line

def parse_generation_data(line):
    """解析Generation行的数据"""
    pattern = r"Generation (\d+).*?fitness: ([\d.e+-]+).*?runtime:([\d.e+-]+).*?error:([\d.e+-]+).*?Chromosome: ([\d\s]+)"
    match = re.search(pattern, line)
    if not match:
        return None
    return {
        "迭代次数": int(match.group(1)),
        "适应度": float(match.group(2)),
        "runtime": float(match.group(3)),
        "error": float(match.group(4)),
        "染色体编码": list(map(int, match.group(5).split()))
    }

def parse_total_time(line):
    """解析总运行时间"""
    match = re.search(r"遗传算法总运行时间: ([\d.]+) 秒", line)
    return float(match.group(1)) if match else None

def parse_content(filepath):
    """主解析函数"""
    # 获取最后两行
    second_last_line, last_line = get_last_two_lines(filepath)
    
    # 解析Generation数据
    generation_data = parse_generation_data(second_last_line)
    if not generation_data:
        return None, None, None, None, None
    
    # 解析总运行时间
    total_time = parse_total_time(last_line)
    
    return (
        generation_data["runtime"],
        generation_data["error"],
        total_time,
        generation_data["迭代次数"],
        generation_data["染色体编码"]
    )

def main():
    log_dir = '/home/zjn/mytool4/log'  # 替换为实际路径
    output_excel = 'result.xlsx'
    
    data = []
    for filename in os.listdir(log_dir):
        if not filename.endswith('.txt'):
            continue
            
        filepath = os.path.join(log_dir, filename)
        e_threshold, mu, cs = parse_filename(filename)
        runtime, error, total_time, iterations, chromosome = parse_content(filepath)
        
        if None in [e_threshold, mu, cs, runtime, error, total_time]:
            print(f"跳过文件 {filename}（数据不完整）")
            continue
        
        data.append({
            "误差阈值": e_threshold,
            "变异概率": mu,
            "交叉概率": cs,
            "Runtime (s)": runtime,
            "Error": error,
            "总运行时间 (s)": total_time,
            "迭代次数": iterations,
            "染色体编码": ' '.join(map(str, chromosome)),  # 转换为字符串便于Excel查看
            "文件名": filename
        })
    
    # 保存为Excel
    df = pd.DataFrame(data)
    df.to_excel(output_excel, index=False, engine='openpyxl')
    print(f"数据已导出至 {output_excel}")

if __name__ == "__main__":
    main()