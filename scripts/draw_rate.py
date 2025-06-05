import matplotlib.pyplot as plt
from collections import defaultdict
import numpy as np
import os
import sys
import argparse

def read_flow_data(filename, start_ms, end_ms):
    # 存储每条流的数据：{flow_id: [(time_0_1ms, rate_gbps), ...]}
    flows = defaultdict(list)
    
    try:
        with open(filename, 'r') as f:
            for line in f:
                # 解析文件行
                src_ip, dst_ip, src_port, dst_port, time_ns, rate_gbps = line.strip().split()
                # 将时间从ns转换为0.1ms（1ms = 10 * 0.1ms）
                time_0_1ms = float(time_ns) / 100_000
                # 流速已经是Gbps，直接转换为float
                rate_gbps = float(rate_gbps)
                # 只保留指定时间范围内的数据（转换为ms后比较）
                time_ms = time_0_1ms / 10  # 0.1ms单位转为ms用于比较
                if start_ms <= time_ms <= end_ms:
                    # 构造流的唯一标识
                    flow_id = f"{src_ip}:{src_port}->{dst_ip}:{dst_port}"
                    # 添加到对应流的列表
                    flows[flow_id].append((time_0_1ms, rate_gbps))
    except FileNotFoundError:
        print(f"错误：文件 {filename} 不存在")
        sys.exit(1)
    except Exception as e:
        print(f"错误：读取文件时发生错误 - {e}")
        sys.exit(1)
    
    return flows

def plot_flow_rates(filename, start_ms, end_ms):
    # 读取数据
    flows = read_flow_data(filename, start_ms, end_ms)
    
    if not flows:
        print(f"警告：在时间范围 {start_ms}ms 到 {end_ms}ms 内没有数据")
        return
    
    # 创建图形
    plt.figure(figsize=(10, 6))
    
    # 为每条流绘制折线
    for flow_id, data in flows.items():
        # 按时间排序
        data.sort(key=lambda x: x[0])
        # 分离时间和流速
        times, rates = zip(*data)
        # 绘制折线
        plt.plot(times, rates, label=flow_id, marker='o')
    
    # 设置图表属性
    plt.xlabel('Time (0.1ms)')
    plt.ylabel('Flow Rate (Gbps)')
    plt.title(f'Flow Rate vs Time ({start_ms}ms to {end_ms}ms)')
    plt.legend()
    plt.grid(True)
    
    # 确保charts文件夹存在
    os.makedirs('./charts', exist_ok=True)
    
    # 保存图形到./charts/文件夹
    output_path = './charts/flow_rate_plot.png'
    plt.savefig(output_path, dpi=300, bbox_inches='tight')
    print(f"图表已保存到 {output_path}")
    
    # 显示图形
    plt.show()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="绘制流速变化曲线图")
    parser.add_argument("filename", help="流速数据文件名")
    parser.add_argument("-d", "--duration", nargs=2, type=float, required=True,
                       metavar=('START_MS', 'END_MS'),
                       help="监测的时间范围（单位：ms），例如：-d 3 5")
    
    args = parser.parse_args()
    
    if args.duration[0] >= args.duration[1]:
        print("错误：开始时间必须小于结束时间")
        sys.exit(1)
    
    plot_flow_rates(args.filename, args.duration[0], args.duration[1])
