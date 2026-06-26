from pathlib import Path
import textwrap

from PIL import Image
from reportlab.lib import colors
from reportlab.lib.enums import TA_CENTER, TA_LEFT
from reportlab.lib.pagesizes import A4, landscape
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib.units import mm
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.pdfgen import canvas
from reportlab.platypus import Paragraph


ROOT = Path(__file__).resolve().parents[1]
OUT_DIR = ROOT / "outputs" / "project_portfolio"
PDF_PATH = OUT_DIR / "等离子电源HMI控制项目_作品集.pdf"
PHOTO_PATH = Path(
    r"C:\Users\99212\Documents\xwechat_files\wxid_4xnyiizsstta12_94b8\temp\RWTemp\2026-06\9e20f478899dc29eb19741386f9343c8\4c7d2212160d39eb8ae6a01fcca43184.jpg"
)

PAGE_W, PAGE_H = landscape(A4)
M = 18 * mm
BLUE = colors.HexColor("#1266A8")
SKY = colors.HexColor("#EAF4FF")
INK = colors.HexColor("#1E2933")
MUTED = colors.HexColor("#64748B")
LINE = colors.HexColor("#C8D6E5")
GREEN = colors.HexColor("#168A5B")
ORANGE = colors.HexColor("#D97706")
RED = colors.HexColor("#D33F49")


def setup_fonts():
    candidates = [
        ("NotoSC", r"C:\Windows\Fonts\NotoSansSC-VF.ttf"),
        ("MicrosoftYaHei", r"C:\Windows\Fonts\msyh.ttc"),
        ("SimHei", r"C:\Windows\Fonts\simhei.ttf"),
    ]
    for name, path in candidates:
        if Path(path).exists():
            pdfmetrics.registerFont(TTFont(name, path))
            return name
    return "Helvetica"


FONT = setup_fonts()
FONT_BOLD = FONT

STYLES = {
    "title": ParagraphStyle(
        "title", fontName=FONT_BOLD, fontSize=29, leading=36, textColor=colors.white,
        alignment=TA_LEFT, wordWrap="CJK"
    ),
    "subtitle": ParagraphStyle(
        "subtitle", fontName=FONT, fontSize=12.5, leading=20, textColor=colors.white,
        alignment=TA_LEFT, wordWrap="CJK"
    ),
    "h1": ParagraphStyle(
        "h1", fontName=FONT_BOLD, fontSize=21, leading=26, textColor=INK,
        spaceAfter=7, wordWrap="CJK"
    ),
    "h2": ParagraphStyle(
        "h2", fontName=FONT_BOLD, fontSize=13.5, leading=18, textColor=BLUE,
        spaceAfter=4, wordWrap="CJK"
    ),
    "body": ParagraphStyle(
        "body", fontName=FONT, fontSize=9.5, leading=15, textColor=INK,
        alignment=TA_LEFT, wordWrap="CJK"
    ),
    "small": ParagraphStyle(
        "small", fontName=FONT, fontSize=8.2, leading=12, textColor=MUTED,
        alignment=TA_LEFT, wordWrap="CJK"
    ),
    "metric": ParagraphStyle(
        "metric", fontName=FONT_BOLD, fontSize=15, leading=18, textColor=BLUE,
        alignment=TA_CENTER, wordWrap="CJK"
    ),
    "metric_label": ParagraphStyle(
        "metric_label", fontName=FONT, fontSize=8.2, leading=11, textColor=MUTED,
        alignment=TA_CENTER, wordWrap="CJK"
    ),
}


def para(c, text, x, y, w, h, style="body"):
    p = Paragraph(text.replace("\n", "<br/>"), STYLES[style])
    _, ph = p.wrap(w, h)
    p.drawOn(c, x, y + h - ph)
    return ph


def footer(c, page_no):
    c.setStrokeColor(LINE)
    c.line(M, 11 * mm, PAGE_W - M, 11 * mm)
    c.setFont(FONT, 7.5)
    c.setFillColor(MUTED)
    c.drawString(M, 6.5 * mm, "STM32F103 · FreeRTOS · USART HMI · PWM/脉冲控制 · 电流采样保护")
    c.drawRightString(PAGE_W - M, 6.5 * mm, f"{page_no:02d}")


def page_title(c, title, subtitle, page_no):
    c.setFillColor(BLUE)
    c.rect(0, PAGE_H - 24 * mm, PAGE_W, 24 * mm, stroke=0, fill=1)
    c.setFillColor(colors.white)
    c.setFont(FONT_BOLD, 18)
    c.drawString(M, PAGE_H - 15 * mm, title)
    c.setFont(FONT, 8.5)
    c.drawRightString(PAGE_W - M, PAGE_H - 15 * mm, subtitle)
    footer(c, page_no)


def rounded_card(c, x, y, w, h, fill=colors.white, stroke=LINE, radius=5):
    c.setFillColor(fill)
    c.setStrokeColor(stroke)
    c.roundRect(x, y, w, h, radius, stroke=1, fill=1)


def bullet_list(c, items, x, y, w, size=9.3, leading=14, color=INK):
    c.setFont(FONT, size)
    c.setFillColor(color)
    cur_y = y
    for item in items:
        wrapped = textwrap.wrap(item, width=36 if w < 95 * mm else 56)
        c.setFillColor(BLUE)
        c.circle(x + 2, cur_y + 3.2, 1.5, stroke=0, fill=1)
        c.setFillColor(color)
        for idx, line in enumerate(wrapped):
            c.drawString(x + 7, cur_y - idx * leading, line)
        cur_y -= max(1, len(wrapped)) * leading + 2
    return cur_y


def metric(c, x, y, w, value, label):
    rounded_card(c, x, y, w, 22 * mm, fill=SKY, stroke=colors.HexColor("#B7D7F5"))
    para(c, value, x + 3 * mm, y + 9 * mm, w - 6 * mm, 10 * mm, "metric")
    para(c, label, x + 3 * mm, y + 3 * mm, w - 6 * mm, 6 * mm, "metric_label")


def fit_image(c, path, x, y, w, h):
    if not path.exists():
        rounded_card(c, x, y, w, h, fill=SKY)
        para(c, "设备照片未找到", x + 8, y + h / 2 - 10, w - 16, 20, "body")
        return
    with Image.open(path) as img:
        iw, ih = img.size
    scale = max(w / iw, h / ih)
    dw, dh = iw * scale, ih * scale
    c.saveState()
    c.rect(x, y, w, h, stroke=0, fill=0)
    c.clipPath(c.beginPath(), stroke=0, fill=0)
    c.drawImage(str(path), x + (w - dw) / 2, y + (h - dh) / 2, dw, dh, preserveAspectRatio=False, mask=None)
    c.restoreState()


def draw_flow(c, x, y, w, h, labels, title=None):
    if title:
        c.setFont(FONT_BOLD, 10)
        c.setFillColor(BLUE)
        c.drawString(x, y + h + 4 * mm, title)
    gap = 8 * mm
    box_w = (w - gap * (len(labels) - 1)) / len(labels)
    for i, (label, sub, color) in enumerate(labels):
        bx = x + i * (box_w + gap)
        rounded_card(c, bx, y, box_w, h, fill=colors.white, stroke=color)
        c.setFillColor(color)
        c.rect(bx, y + h - 5 * mm, box_w, 5 * mm, stroke=0, fill=1)
        para(c, label, bx + 3 * mm, y + h - 18 * mm, box_w - 6 * mm, 12 * mm, "h2")
        para(c, sub, bx + 3 * mm, y + 4 * mm, box_w - 6 * mm, h - 20 * mm, "small")
        if i < len(labels) - 1:
            c.setStrokeColor(MUTED)
            c.line(bx + box_w + 2 * mm, y + h / 2, bx + box_w + gap - 2 * mm, y + h / 2)
            c.line(bx + box_w + gap - 3 * mm, y + h / 2 + 2, bx + box_w + gap - 2 * mm, y + h / 2)
            c.line(bx + box_w + gap - 3 * mm, y + h / 2 - 2, bx + box_w + gap - 2 * mm, y + h / 2)


def draw_ui_mock(c, x, y, w, h):
    rounded_card(c, x, y, w, h, fill=colors.HexColor("#F8FAFC"), stroke=LINE, radius=2)
    c.setFont(FONT_BOLD, 12)
    c.setFillColor(INK)
    c.drawString(x + 8 * mm, y + h - 12 * mm, "MODE SET")
    c.drawCentredString(x + w / 2, y + h - 12 * mm, "TEST SET")
    c.drawRightString(x + w - 12 * mm, y + h - 12 * mm, "OUTPUT")
    for i, label in enumerate(["FREQUENCY", "DUTY CYCLE", "COUNTS"]):
        yy = y + h - (24 + i * 20) * mm
        c.setFont(FONT, 7.8)
        c.drawCentredString(x + w / 2, yy + 8 * mm, label)
        c.setFillColor(colors.HexColor("#8DB5E0"))
        c.rect(x + w / 2 - 16 * mm, yy, 32 * mm, 8 * mm, stroke=0, fill=1)
        c.setFillColor(INK)
        c.setFont(FONT, 10)
        c.drawCentredString(x + w / 2, yy + 2.2 * mm, "0")
    for i, label in enumerate(["VOLTAGE", "CURRENT"]):
        yy = y + h - (24 + i * 20) * mm
        c.setFont(FONT, 7.8)
        c.drawCentredString(x + w - 30 * mm, yy + 8 * mm, label)
        c.setFillColor(colors.HexColor("#8DB5E0"))
        c.rect(x + w - 46 * mm, yy, 32 * mm, 8 * mm, stroke=0, fill=1)
        c.setFillColor(INK)
        c.setFont(FONT, 10)
        c.drawCentredString(x + w - 30 * mm, yy + 2.2 * mm, "0.00")
    c.setStrokeColor(colors.black)
    c.setLineWidth(1.5)
    c.rect(x + 11 * mm, y + h - 30 * mm, 9 * mm, 6 * mm, stroke=1, fill=0)
    c.line(x + 15.5 * mm, y + h - 30 * mm, x + 15.5 * mm, y + h - 39 * mm)
    c.rect(x + 11 * mm, y + 15 * mm, 14 * mm, 9 * mm, stroke=1, fill=0)
    c.arc(x + 13 * mm, y + 18 * mm, x + 23 * mm, y + 31 * mm, 200, 140)
    c.setStrokeColor(colors.HexColor("#888888"))
    c.setLineWidth(5)
    c.arc(x + w - 44 * mm, y + 13 * mm, x + w - 27 * mm, y + 30 * mm, 130, 280)
    c.line(x + w - 35.5 * mm, y + 29 * mm, x + w - 35.5 * mm, y + 38 * mm)


def cover(c):
    c.setFillColor(BLUE)
    c.rect(0, 0, PAGE_W, PAGE_H, stroke=0, fill=1)
    fit_image(c, PHOTO_PATH, PAGE_W * 0.43, 0, PAGE_W * 0.57, PAGE_H)
    c.setFillColor(colors.Color(0, 0, 0, alpha=0.18))
    c.rect(PAGE_W * 0.43, 0, PAGE_W * 0.57, PAGE_H, stroke=0, fill=1)
    para(c, "等离子电源 HMI 控制系统", M, PAGE_H - 63 * mm, PAGE_W * 0.38, 32 * mm, "title")
    para(c, "STM32F103 + FreeRTOS 实时控制项目作品集<br/>应聘方向：嵌入式软件 / 运动控制 / 工业设备控制", M, PAGE_H - 94 * mm, PAGE_W * 0.36, 30 * mm, "subtitle")
    c.setFillColor(colors.white)
    c.setStrokeColor(colors.white)
    c.line(M, PAGE_H - 104 * mm, M + 68 * mm, PAGE_H - 104 * mm)
    c.setFont(FONT, 9)
    c.drawString(M, 38 * mm, "候选人：待补充")
    c.drawString(M, 31 * mm, "项目周期：2025-2026 · 工具链：STM32CubeIDE / HAL / CMSIS-RTOS2")
    c.drawString(M, 24 * mm, "交付内容：可调 PWM 输出、定脉冲控制、串口屏交互、电流采样与保护")


def page_overview(c):
    page_title(c, "01 项目概览", "从控制面板到实时输出的完整嵌入式闭环", 2)
    para(c, "项目目标", M, PAGE_H - 43 * mm, 80 * mm, 12 * mm, "h1")
    para(c, "为等离子/脉冲输出设备设计一套基于 STM32F103 的控制程序：用户通过串口触摸屏设置频率、占空比与脉冲数，系统根据脚踏开关与档位输入控制单极/双极 PWM 输出，同时采集电流并在异常时快速关断。", M, PAGE_H - 67 * mm, 118 * mm, 25 * mm, "body")
    metric(c, M, PAGE_H - 96 * mm, 40 * mm, "4", "FreeRTOS 任务")
    metric(c, M + 45 * mm, PAGE_H - 96 * mm, 44 * mm, "400", "ADC DMA 双缓冲点数")
    metric(c, M + 94 * mm, PAGE_H - 96 * mm, 46 * mm, "20-120 kHz", "PWM 频率限制")
    metric(c, M + 145 * mm, PAGE_H - 96 * mm, 37 * mm, "10 A", "过流保护阈值")
    draw_ui_mock(c, PAGE_W - M - 98 * mm, PAGE_H - 116 * mm, 98 * mm, 70 * mm)
    para(c, "我的工作", M, PAGE_H - 123 * mm, 80 * mm, 12 * mm, "h1")
    bullet_list(c, [
        "搭建 STM32CubeMX/HAL 工程，完成 ADC、TIM、USART、GPIO、IWDG 与 FreeRTOS 配置。",
        "设计串口屏协议解析流程，支持开关机、频率、占空比、脉冲数等指令输入。",
        "实现 PWM 安全启停、单极/双极档位输出、脚踏控制与定脉冲输出互斥。",
        "完成 ACS713/ACS712 类霍尔电流采样、零点校准、RMS 计算、滤波与过流保护。",
    ], M, PAGE_H - 136 * mm, 128 * mm)
    para(c, "面试中可突出：这不是单个外设 demo，而是一个有输入、输出、显示、保护、任务调度和异常恢复的设备控制小系统。", PAGE_W - M - 98 * mm, 25 * mm, 98 * mm, 18 * mm, "small")


def page_arch(c):
    page_title(c, "02 系统架构", "硬件输入、实时任务与输出执行链路", 3)
    draw_flow(c, M, PAGE_H - 74 * mm, PAGE_W - 2 * M, 30 * mm, [
        ("触摸屏/HMI", "USART2 接收 A/B 开关机与 0x55 帧格式参数指令", BLUE),
        ("控制任务", "消息队列接收脉冲数；脚踏状态消抖；档位选择", GREEN),
        ("PWM/脉冲模块", "TIM1 互补 PWM 输出；ARR/CCR 动态更新；定脉冲自动停止", ORANGE),
        ("执行端", "单极/双极输出通道，停止时 GPIO 强制安全电平", RED),
    ], "主控制链路")
    draw_flow(c, M, PAGE_H - 130 * mm, PAGE_W - 2 * M, 30 * mm, [
        ("ADC DMA", "400 点循环缓冲，半满/满中断同步处理", BLUE),
        ("电流任务", "RMS 计算 + 死区 + 一阶滤波，互斥更新显示值", GREEN),
        ("保护策略", "电流超过阈值立刻 PWM_Stop，清除系统使能", RED),
        ("UI/蓝牙", "低优先级周期刷新，异常时线程标志立即唤醒", ORANGE),
    ], "采样保护链路")
    rounded_card(c, M, 27 * mm, PAGE_W - 2 * M, 32 * mm, fill=colors.HexColor("#FBFDFF"))
    para(c, "架构亮点", M + 6 * mm, 48 * mm, 40 * mm, 8 * mm, "h2")
    bullet_list(c, [
        "中断中只做轻量接收与投递，耗时动作下放到任务，降低串口回调阻塞风险。",
        "高频采样处理与 UI 刷新解耦，电流保护优先级高于显示美观。",
        "PWM 停止路径包含主输出使能关闭、通道停止、GPIO 安全电平三层处理。"
    ], M + 6 * mm, 42 * mm, PAGE_W - 2 * M - 12 * mm, size=8.8, leading=12)


def page_realtime(c):
    page_title(c, "03 FreeRTOS 实时任务设计", "让采样、控制、显示和看门狗各司其职", 4)
    tasks = [
        ("Task_Current", "High", "ADC DMA 半满/满信号触发，计算 RMS 电流并执行过流急停。"),
        ("Task_Control", "AboveNormal", "接收 PulseQueue，处理脚踏开关、档位与定脉冲输出互斥。"),
        ("Task_UI_Comm", "Normal", "周期刷新屏幕与蓝牙数据；异常时由线程标志立即唤醒。"),
        ("Task_Watchdog", "Realtime", "等待三个任务打卡，全部在线才刷新 IWDG。"),
    ]
    x = M
    y = PAGE_H - 58 * mm
    col_w = (PAGE_W - 2 * M - 9 * mm) / 4
    for i, (name, prio, desc) in enumerate(tasks):
        bx = x + i * (col_w + 3 * mm)
        rounded_card(c, bx, y - 51 * mm, col_w, 51 * mm, fill=colors.white)
        c.setFillColor(BLUE if i != 3 else RED)
        c.rect(bx, y - 8 * mm, col_w, 8 * mm, stroke=0, fill=1)
        c.setFillColor(colors.white)
        c.setFont(FONT_BOLD, 9.5)
        c.drawCentredString(bx + col_w / 2, y - 5.3 * mm, name)
        para(c, f"优先级：{prio}", bx + 4 * mm, y - 21 * mm, col_w - 8 * mm, 8 * mm, "h2")
        para(c, desc, bx + 4 * mm, y - 47 * mm, col_w - 8 * mm, 24 * mm, "small")
    para(c, "同步机制", M, PAGE_H - 126 * mm, 60 * mm, 10 * mm, "h1")
    bullet_list(c, [
        "信号量：Sem_ADC_Half / Sem_ADC_Full 对应 DMA 双缓冲，避免主循环轮询。",
        "消息队列：PulseQueue 将 HMI 的脉冲数命令投递给控制任务，避免在串口中断里直接执行。",
        "互斥锁：Mutex_Current 保护 current_display，UI 任务只短时间读取快照。",
        "事件标志：EventGroup_WDG 汇总任务健康状态，防止某个任务卡死仍持续喂狗。"
    ], M, PAGE_H - 139 * mm, 123 * mm, size=9, leading=13)
    rounded_card(c, PAGE_W - M - 92 * mm, 33 * mm, 92 * mm, 60 * mm, fill=SKY, stroke=colors.HexColor("#B7D7F5"))
    para(c, "可讲述的工程取舍", PAGE_W - M - 86 * mm, 79 * mm, 80 * mm, 10 * mm, "h2")
    para(c, "UI 刷新并不需要高频运行，因此放在 Normal 优先级并设置 200ms 等待；但过流发生时，Current 任务会立即设置线程标志唤醒 UI。这兼顾了实时保护和人机界面响应。", PAGE_W - M - 86 * mm, 39 * mm, 80 * mm, 36 * mm, "body")


def page_modules(c):
    page_title(c, "04 关键模块实现", "PWM、安全停止、电流算法与 HMI 协议", 5)
    boxes = [
        ("PWM 输出", "TIM1 互补 PWM；根据硬件档位开启单极/双极通道；动态修改 ARR/CCR 时保留当前占空比并做边界钳位。", BLUE),
        ("定脉冲控制", "PulseControl_Start 设置目标脉冲数，TIM1 更新中断递增计数，到达目标自动 Stop；运行中拒绝重复启动。", GREEN),
        ("电流检测", "上电延时后做零点校准；200 点一组计算 RMS；使用整数平方和与整数开方减少 F103 浮点开销。", ORANGE),
        ("滤波保护", "ADC 层死区 + 一阶滞后滤波抑制底噪；超过阈值立即关闭 PWM 并清除 system_enable。", RED),
        ("HMI 协议", "支持 A/B 开关机；0x55 + 类型 + 4 字节数据 + 0D 0A 参数帧；频率、占空比、脉冲数分类型处理。", BLUE),
        ("安全输出态", "停止时禁用 TIM 主输出、停止所有通道，再把 PWM 引脚切回普通输出并强制拉低。", RED),
    ]
    for i, (title, body, color) in enumerate(boxes):
        col = i % 3
        row = i // 3
        bw = (PAGE_W - 2 * M - 10 * mm) / 3
        bh = 50 * mm
        bx = M + col * (bw + 5 * mm)
        by = PAGE_H - 74 * mm - row * (bh + 10 * mm)
        rounded_card(c, bx, by, bw, bh, fill=colors.white, stroke=color)
        c.setFillColor(color)
        c.rect(bx, by + bh - 6 * mm, bw, 6 * mm, stroke=0, fill=1)
        para(c, title, bx + 4 * mm, by + bh - 20 * mm, bw - 8 * mm, 10 * mm, "h2")
        para(c, body, bx + 4 * mm, by + 7 * mm, bw - 8 * mm, 28 * mm, "small")
    rounded_card(c, M, 20 * mm, PAGE_W - 2 * M, 28 * mm, fill=colors.HexColor("#FFF8ED"), stroke=colors.HexColor("#F3C27A"))
    para(c, "投递版表达", M + 6 * mm, 37 * mm, 35 * mm, 8 * mm, "h2")
    para(c, "我把这个项目包装为“实时工业设备控制系统”：既能说明底层外设，也能展示任务调度、异常保护、通信协议、算法优化和硬件联调能力。面试时不建议只说“会 STM32”，而要强调你能把多个外设组织成可靠系统。", M + 42 * mm, 24 * mm, PAGE_W - 2 * M - 50 * mm, 20 * mm, "body")


def page_result(c):
    page_title(c, "05 项目成果与面试讲述", "把作品转换成面试官能快速判断的能力信号", 6)
    fit_image(c, PHOTO_PATH, M, PAGE_H - 103 * mm, 120 * mm, 68 * mm)
    rounded_card(c, M + 126 * mm, PAGE_H - 103 * mm, PAGE_W - 2 * M - 126 * mm, 68 * mm, fill=colors.white)
    para(c, "推荐 60 秒项目介绍", M + 132 * mm, PAGE_H - 49 * mm, 80 * mm, 8 * mm, "h2")
    para(c, "这个项目是我基于 STM32F103 做的一套等离子电源 HMI 控制系统。用户可以在串口屏上设置频率、占空比和脉冲数，系统通过 TIM1 输出互补 PWM，并根据硬件档位切换单极/双极模式。我把程序拆成电流采样、控制、UI 通信和看门狗四个 FreeRTOS 任务，用 DMA 双缓冲处理电流采样，用消息队列把 HMI 的脉冲命令交给控制任务，避免在中断里做耗时操作。安全上实现了过流急停、脚踏控制互斥、PWM 停止后的 GPIO 安全态以及任务级看门狗打卡。这个项目锻炼了我从外设配置、实时任务设计到硬件联调和异常保护的完整嵌入式开发能力。", M + 132 * mm, PAGE_H - 97 * mm, PAGE_W - 2 * M - 138 * mm, 45 * mm, "body")
    para(c, "面试可追问点", M, PAGE_H - 124 * mm, 70 * mm, 10 * mm, "h1")
    bullet_list(c, [
        "为什么 ADC 用 DMA 双缓冲，而不是循环里 HAL_ADC_PollForConversion？",
        "为什么串口中断里只收帧和投递队列，不直接启动定脉冲？",
        "PWM_SetFreq 如何保持原占空比，避免参数更新造成异常输出？",
        "过流保护为什么放在电流任务里先判断，再更新 UI 显示？",
        "IWDG 为什么不能由单一任务无条件刷新？"
    ], M, PAGE_H - 138 * mm, 105 * mm, size=9, leading=13)
    rounded_card(c, PAGE_W - M - 122 * mm, 25 * mm, 122 * mm, 57 * mm, fill=SKY, stroke=colors.HexColor("#B7D7F5"))
    para(c, "建议补充材料", PAGE_W - M - 116 * mm, 68 * mm, 80 * mm, 8 * mm, "h2")
    bullet_list(c, [
        "示波器截图：PWM 频率、占空比、单极/双极输出波形。",
        "实测表格：频率误差、电流显示稳定性、过流触发时间。",
        "短视频：触摸屏设置参数、脚踏启动、定脉冲自动停止。"
    ], PAGE_W - M - 116 * mm, 60 * mm, 105 * mm, size=8.5, leading=12)


def build():
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    c = canvas.Canvas(str(PDF_PATH), pagesize=landscape(A4))
    cover(c)
    c.showPage()
    for page in [page_overview, page_arch, page_realtime, page_modules, page_result]:
        page(c)
        c.showPage()
    c.save()
    print(PDF_PATH)


if __name__ == "__main__":
    build()
