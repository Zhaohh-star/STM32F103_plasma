import fs from "node:fs/promises";
import path from "node:path";
import { fileURLToPath } from "node:url";

const artifactPath =
  "file:///C:/Users/99212/.cache/codex-runtimes/codex-primary-runtime/dependencies/node/node_modules/@oai/artifact-tool/dist/artifact_tool.mjs";
const { Presentation, PresentationFile } = await import(artifactPath);

const __filename = fileURLToPath(import.meta.url);
const ROOT = path.resolve(path.dirname(__filename), "..");
const OUT_DIR = path.join(ROOT, "outputs", "project_portfolio");
const PREVIEW_DIR = path.join(OUT_DIR, "pptx_preview");
const PPTX_PATH = path.join(OUT_DIR, "等离子电源HMI控制项目_作品集.pptx");
const PHOTO_PATH = path.join(OUT_DIR, "device_photo.jpg");
const APPENDIX_1_PATH = path.join(OUT_DIR, "appendix_device_platform.png");
const APPENDIX_2_PATH = path.join(OUT_DIR, "appendix_current_sampling.png");

const W = 1280;
const H = 720;
const BLUE = "#1266A8";
const SKY = "#EAF4FF";
const INK = "#1E2933";
const MUTED = "#64748B";
const LINE = "#C8D6E5";
const GREEN = "#168A5B";
const ORANGE = "#D97706";
const RED = "#D33F49";
const FONT = "Microsoft YaHei";
let photoData = null;
let appendix1Data = null;
let appendix2Data = null;

function rect(slide, x, y, w, h, fill = "#FFFFFF", line = LINE, radius = 8) {
  const shape = slide.shapes.add({
    geometry: "rect",
    position: { left: x, top: y, width: w, height: h },
    fill,
    line: { fill: line, width: 1 },
  });
  shape.borderRadius = radius;
  return shape;
}

function text(slide, value, x, y, w, h, opts = {}) {
  const shape = slide.shapes.add({
    geometry: "rect",
    position: { left: x, top: y, width: w, height: h },
    fill: "none",
    line: { fill: "none" },
  });
  shape.text.set(value);
  shape.text.typeface = FONT;
  shape.text.fontSize = opts.size ?? 22;
  shape.text.color = opts.color ?? INK;
  shape.text.bold = opts.bold ?? false;
  shape.text.alignment = opts.align ?? "left";
  shape.text.verticalAlignment = opts.valign ?? "top";
  shape.text.wrap = "square";
  shape.text.insets = { top: 0, right: 0, bottom: 0, left: 0 };
  if (opts.leading) shape.text.lineSpacing = opts.leading;
  return shape;
}

function title(slide, page, label) {
  rect(slide, 0, 0, W, 78, BLUE, BLUE, 0);
  text(slide, page, 48, 24, 560, 36, { size: 30, bold: true, color: "#FFFFFF" });
  text(slide, label, W - 458, 29, 410, 26, { size: 15, color: "#FFFFFF", align: "right" });
  text(slide, "STM32F103 · FreeRTOS · USART HMI · PWM/脉冲控制 · 电流采样保护", 48, H - 34, 680, 18, {
    size: 12,
    color: MUTED,
  });
}

function bullet(slide, items, x, y, w, opts = {}) {
  let cy = y;
  for (const item of items) {
    slide.shapes.add({
      geometry: "ellipse",
      position: { left: x, top: cy + 7, width: 7, height: 7 },
      fill: opts.dot ?? BLUE,
      line: { fill: opts.dot ?? BLUE },
    });
    const lines = item.length > 34 ? Math.ceil(item.length / 34) : 1;
    text(slide, item, x + 18, cy, w - 18, 22 * lines, {
      size: opts.size ?? 17,
      color: opts.color ?? INK,
      leading: 1.05,
    });
    cy += 28 + (lines - 1) * 18;
  }
}

function metric(slide, x, y, w, value, label) {
  rect(slide, x, y, w, 74, SKY, "#B7D7F5", 8);
  text(slide, value, x, y + 14, w, 28, { size: 26, bold: true, color: BLUE, align: "center" });
  text(slide, label, x + 8, y + 48, w - 16, 18, { size: 13, color: MUTED, align: "center" });
}

function flow(slide, x, y, w, h, steps) {
  const gap = 22;
  const bw = (w - gap * (steps.length - 1)) / steps.length;
  steps.forEach((s, i) => {
    const bx = x + i * (bw + gap);
    rect(slide, bx, y, bw, h, "#FFFFFF", s.color, 8);
    rect(slide, bx, y, bw, 12, s.color, s.color, 0);
    text(slide, s.title, bx + 14, y + 24, bw - 28, 26, { size: 19, bold: true, color: s.color });
    text(slide, s.body, bx + 14, y + 58, bw - 28, h - 66, { size: 13, color: MUTED });
    if (i < steps.length - 1) {
      slide.shapes.add({
        geometry: "rect",
        position: { left: bx + bw + 4, top: y + h / 2 - 1, width: gap - 12, height: 2 },
        fill: MUTED,
        line: { fill: MUTED },
      });
      slide.shapes.add({
        geometry: "triangle",
        position: { left: bx + bw + gap - 12, top: y + h / 2 - 6, width: 10, height: 12 },
        fill: MUTED,
        line: { fill: MUTED },
      });
    }
  });
}

function uiMock(slide, x, y, w, h) {
  rect(slide, x, y, w, h, "#F8FAFC", LINE, 4);
  text(slide, "MODE SET", x + 26, y + 24, 160, 26, { size: 20, bold: true, color: INK });
  text(slide, "TEST SET", x + w / 2 - 80, y + 24, 160, 26, { size: 20, bold: true, color: INK, align: "center" });
  text(slide, "OUTPUT", x + w - 174, y + 24, 140, 26, { size: 20, bold: true, color: INK, align: "center" });
  ["FREQUENCY", "DUTY CYCLE", "COUNTS"].forEach((label, i) => {
    const yy = y + 72 + i * 70;
    text(slide, label, x + w / 2 - 75, yy, 150, 18, { size: 12, color: INK, align: "center" });
    rect(slide, x + w / 2 - 70, yy + 22, 140, 32, "#8DB5E0", "#8DB5E0", 0);
    text(slide, "0", x + w / 2 - 70, yy + 25, 140, 24, { size: 18, color: INK, align: "center" });
  });
  ["VOLTAGE", "CURRENT"].forEach((label, i) => {
    const yy = y + 72 + i * 70;
    text(slide, label, x + w - 176, yy, 140, 18, { size: 12, color: INK, align: "center" });
    rect(slide, x + w - 174, yy + 22, 140, 32, "#8DB5E0", "#8DB5E0", 0);
    text(slide, "0.00", x + w - 174, yy + 25, 140, 24, { size: 18, color: INK, align: "center" });
  });
  rect(slide, x + 46, y + 80, 44, 32, "none", "#000000", 0);
  rect(slide, x + 42, y + h - 92, 62, 44, "none", "#000000", 0);
  slide.shapes.add({
    geometry: "ellipse",
    position: { left: x + w - 160, top: y + h - 86, width: 78, height: 78 },
    line: { fill: "#888888", width: 9 },
    fill: "none",
  });
  slide.shapes.add({
    geometry: "rect",
    position: { left: x + w - 125, top: y + h - 92, width: 10, height: 42 },
    fill: "#888888",
    line: { fill: "#888888" },
  });
}

async function addCover(pres) {
  const slide = pres.slides.add();
  rect(slide, 0, 0, W, H, BLUE, BLUE, 0);
  slide.images.add({
    data: photoData,
    position: { left: 548, top: 0, width: 732, height: 720 },
    fit: "cover",
    alt: "设备实物照片",
  });
  text(slide, "等离子电源 HMI 控制系统", 62, 128, 450, 96, { size: 44, bold: true, color: "#FFFFFF" });
  text(slide, "STM32F103 + FreeRTOS 实时控制项目作品集\n应聘方向：嵌入式软件 / 运动控制 / 工业设备控制", 66, 248, 430, 72, {
    size: 20,
    color: "#FFFFFF",
  });
  rect(slide, 66, 360, 300, 3, "#FFFFFF", "#FFFFFF", 0);
  text(slide, "候选人：待补充", 66, 545, 320, 26, { size: 17, color: "#FFFFFF" });
  text(slide, "项目周期：2025-2026\n工具链：STM32CubeIDE / HAL / CMSIS-RTOS2", 66, 580, 430, 42, {
    size: 13,
    color: "#FFFFFF",
  });
  text(slide, "交付内容：可调 PWM 输出、定脉冲控制、串口屏交互、电流采样与保护", 66, 632, 430, 24, {
    size: 13,
    color: "#FFFFFF",
  });
}

function addOverview(pres) {
  const slide = pres.slides.add();
  title(slide, "01 项目概览", "从控制面板到实时输出的完整嵌入式闭环");
  text(slide, "项目目标", 58, 108, 180, 34, { size: 30, bold: true, color: INK });
  text(
    slide,
    "为等离子/脉冲输出设备设计一套基于 STM32F103 的控制程序：用户通过串口触摸屏设置频率、占空比与脉冲数，系统根据脚踏开关与档位输入控制单极/双极 PWM 输出，同时采集电流并在异常时快速关断。",
    60,
    156,
    700,
    86,
    { size: 18, color: INK },
  );
  metric(slide, 60, 270, 150, "4", "FreeRTOS 任务");
  metric(slide, 232, 270, 170, "400", "ADC DMA 双缓冲点数");
  metric(slide, 424, 270, 185, "20-120 kHz", "PWM 频率限制");
  metric(slide, 632, 270, 140, "10 A", "过流保护阈值");
  uiMock(slide, 820, 116, 400, 300);
  text(slide, "我的工作", 60, 394, 160, 34, { size: 30, bold: true, color: INK });
  bullet(slide, [
    "搭建 STM32CubeMX/HAL 工程，完成 ADC、TIM、USART、GPIO、IWDG 与 FreeRTOS 配置。",
    "设计串口屏协议解析流程，支持开关机、频率、占空比、脉冲数等指令输入。",
    "实现 PWM 安全启停、单极/双极档位输出、脚踏控制与定脉冲输出互斥。",
    "完成 ACS713/ACS712 类霍尔电流采样、零点校准、RMS 计算、滤波与过流保护。",
  ], 66, 450, 720, { size: 17 });
  rect(slide, 820, 460, 400, 95, SKY, "#B7D7F5", 8);
  text(slide, "面试中可突出", 842, 482, 180, 26, { size: 19, bold: true, color: BLUE });
  text(slide, "这不是单个外设 demo，而是一个有输入、输出、显示、保护、任务调度和异常恢复的设备控制小系统。", 842, 520, 350, 38, {
    size: 15,
    color: INK,
  });
}

function addArchitecture(pres) {
  const slide = pres.slides.add();
  title(slide, "02 系统架构", "硬件输入、实时任务与输出执行链路");
  text(slide, "主控制链路", 58, 112, 180, 28, { size: 24, bold: true, color: BLUE });
  flow(slide, 58, 156, 1164, 118, [
    { title: "触摸屏/HMI", body: "USART2 接收 A/B 开关机与 0x55 帧格式参数指令", color: BLUE },
    { title: "控制任务", body: "消息队列接收脉冲数；脚踏状态消抖；档位选择", color: GREEN },
    { title: "PWM/脉冲模块", body: "TIM1 互补 PWM 输出；ARR/CCR 动态更新；定脉冲自动停止", color: ORANGE },
    { title: "执行端", body: "单极/双极输出通道，停止时 GPIO 强制安全电平", color: RED },
  ]);
  text(slide, "采样保护链路", 58, 322, 200, 28, { size: 24, bold: true, color: BLUE });
  flow(slide, 58, 366, 1164, 118, [
    { title: "ADC DMA", body: "400 点循环缓冲，半满/满中断同步处理", color: BLUE },
    { title: "电流任务", body: "RMS 计算 + 死区 + 一阶滤波，互斥更新显示值", color: GREEN },
    { title: "保护策略", body: "电流超过阈值立刻 PWM_Stop，清除系统使能", color: RED },
    { title: "UI/蓝牙", body: "低优先级周期刷新，异常时线程标志立即唤醒", color: ORANGE },
  ]);
  rect(slide, 58, 542, 1164, 88, "#FBFDFF", LINE, 8);
  text(slide, "架构亮点", 84, 562, 140, 24, { size: 20, bold: true, color: BLUE });
  bullet(slide, [
    "中断中只做轻量接收与投递，耗时动作下放到任务，降低串口回调阻塞风险。",
    "高频采样处理与 UI 刷新解耦，电流保护优先级高于显示美观。",
    "PWM 停止路径包含主输出使能关闭、通道停止、GPIO 安全电平三层处理。",
  ], 250, 560, 900, { size: 14 });
}

function addRealtime(pres) {
  const slide = pres.slides.add();
  title(slide, "03 FreeRTOS 实时任务设计", "让采样、控制、显示和看门狗各司其职");
  const tasks = [
    ["Task_Current", "High", "ADC DMA 半满/满信号触发，计算 RMS 电流并执行过流急停。", BLUE],
    ["Task_Control", "AboveNormal", "接收 PulseQueue，处理脚踏开关、档位与定脉冲输出互斥。", GREEN],
    ["Task_UI_Comm", "Normal", "周期刷新屏幕与蓝牙数据；异常时由线程标志立即唤醒。", ORANGE],
    ["Task_Watchdog", "Realtime", "等待三个任务打卡，全部在线才刷新 IWDG。", RED],
  ];
  tasks.forEach((t, i) => {
    const x = 58 + i * 294;
    rect(slide, x, 122, 270, 166, "#FFFFFF", t[3], 8);
    rect(slide, x, 122, 270, 28, t[3], t[3], 0);
    text(slide, t[0], x, 128, 270, 20, { size: 18, bold: true, color: "#FFFFFF", align: "center" });
    text(slide, `优先级：${t[1]}`, x + 18, 170, 220, 24, { size: 17, bold: true, color: t[3] });
    text(slide, t[2], x + 18, 208, 230, 60, { size: 14, color: MUTED });
  });
  text(slide, "同步机制", 58, 340, 160, 34, { size: 30, bold: true, color: INK });
  bullet(slide, [
    "信号量：Sem_ADC_Half / Sem_ADC_Full 对应 DMA 双缓冲，避免主循环轮询。",
    "消息队列：PulseQueue 将 HMI 的脉冲数命令投递给控制任务，避免在串口中断里直接执行。",
    "互斥锁：Mutex_Current 保护 current_display，UI 任务只短时间读取快照。",
    "事件标志：EventGroup_WDG 汇总任务健康状态，防止某个任务卡死仍持续喂狗。",
  ], 66, 396, 730, { size: 16 });
  rect(slide, 840, 370, 382, 150, SKY, "#B7D7F5", 8);
  text(slide, "可讲述的工程取舍", 866, 398, 240, 26, { size: 20, bold: true, color: BLUE });
  text(slide, "UI 刷新并不需要高频运行，因此放在 Normal 优先级并设置 200ms 等待；但过流发生时，Current 任务会立即设置线程标志唤醒 UI。这兼顾了实时保护和人机界面响应。", 866, 442, 320, 78, {
    size: 15,
    color: INK,
  });
}

function addModules(pres) {
  const slide = pres.slides.add();
  title(slide, "04 关键模块实现", "PWM、安全停止、电流算法与 HMI 协议");
  const cards = [
    ["PWM 输出", "TIM1 互补 PWM；根据硬件档位开启单极/双极通道；动态修改 ARR/CCR 时保留当前占空比并做边界钳位。", BLUE],
    ["定脉冲控制", "PulseControl_Start 设置目标脉冲数，TIM1 更新中断递增计数，到达目标自动 Stop；运行中拒绝重复启动。", GREEN],
    ["电流检测", "上电延时后做零点校准；200 点一组计算 RMS；使用整数平方和与整数开方减少 F103 浮点开销。", ORANGE],
    ["滤波保护", "ADC 层死区 + 一阶滞后滤波抑制底噪；超过阈值立即关闭 PWM 并清除 system_enable。", RED],
    ["HMI 协议", "支持 A/B 开关机；0x55 + 类型 + 4 字节数据 + 0D 0A 参数帧；频率、占空比、脉冲数分类型处理。", BLUE],
    ["安全输出态", "停止时禁用 TIM 主输出、停止所有通道，再把 PWM 引脚切回普通输出并强制拉低。", RED],
  ];
  cards.forEach((c, i) => {
    const col = i % 3;
    const row = Math.floor(i / 3);
    const x = 58 + col * 394;
    const y = 118 + row * 180;
    rect(slide, x, y, 360, 142, "#FFFFFF", c[2], 8);
    rect(slide, x, y, 360, 14, c[2], c[2], 0);
    text(slide, c[0], x + 18, y + 32, 240, 24, { size: 20, bold: true, color: c[2] });
    text(slide, c[1], x + 18, y + 70, 315, 54, { size: 14, color: MUTED });
  });
  rect(slide, 58, 512, 1164, 86, "#FFF8ED", "#F3C27A", 8);
  text(slide, "投递版表达", 86, 540, 160, 24, { size: 20, bold: true, color: ORANGE });
  text(slide, "把这个项目包装为“实时工业设备控制系统”：既能说明底层外设，也能展示任务调度、异常保护、通信协议、算法优化和硬件联调能力。面试时不建议只说“会 STM32”，而要强调能把多个外设组织成可靠系统。", 260, 534, 875, 42, {
    size: 16,
    color: INK,
  });
}

function addTalkTrack(pres) {
  const slide = pres.slides.add();
  title(slide, "05 项目成果与面试讲述", "把作品转换成面试官能快速判断的能力信号");
  slide.images.add({
    data: photoData,
    position: { left: 58, top: 126, width: 510, height: 300 },
    fit: "cover",
    alt: "设备实物照片",
  });
  rect(slide, 610, 126, 612, 300, "#FFFFFF", LINE, 8);
  text(slide, "推荐 60 秒项目介绍", 638, 154, 280, 28, { size: 22, bold: true, color: BLUE });
  text(slide, "这个项目是我基于 STM32F103 做的一套等离子电源 HMI 控制系统。用户可以在串口屏上设置频率、占空比和脉冲数，系统通过 TIM1 输出互补 PWM，并根据硬件档位切换单极/双极模式。我把程序拆成电流采样、控制、UI 通信和看门狗四个 FreeRTOS 任务，用 DMA 双缓冲处理电流采样，用消息队列把 HMI 的脉冲命令交给控制任务，避免在中断里做耗时操作。安全上实现了过流急停、脚踏控制互斥、PWM 停止后的 GPIO 安全态以及任务级看门狗打卡。", 638, 198, 540, 170, {
    size: 15,
    color: INK,
  });
  text(slide, "面试可追问点", 58, 472, 220, 30, { size: 26, bold: true, color: INK });
  bullet(slide, [
    "为什么 ADC 用 DMA 双缓冲，而不是循环里 HAL_ADC_PollForConversion？",
    "为什么串口中断里只收帧和投递队列，不直接启动定脉冲？",
    "PWM_SetFreq 如何保持原占空比，避免参数更新造成异常输出？",
    "IWDG 为什么不能由单一任务无条件刷新？",
  ], 66, 522, 640, { size: 15 });
  rect(slide, 820, 478, 402, 118, SKY, "#B7D7F5", 8);
  text(slide, "建议补充材料", 846, 502, 180, 24, { size: 20, bold: true, color: BLUE });
  bullet(slide, [
    "示波器截图：PWM 频率、占空比、单极/双极输出波形。",
    "实测表格：频率误差、电流显示稳定性、过流触发时间。",
    "短视频：触摸屏设置参数、脚踏启动、定脉冲自动停止。",
  ], 846, 540, 330, { size: 13 });
}

function addAppendixSlideOne(pres) {
  const slide = pres.slides.add();
  title(slide, "06 设备模块展示", "等离子电源模块与纹影实验平台");
  slide.images.add({
    data: appendix1Data,
    position: { left: 48, top: 110, width: 1184, height: 516 },
    fit: "contain",
    alt: "等离子电源模块与纹影实验平台",
  });
  text(slide, "左侧：等离子电源模块  右侧：纹影实验平台", 70, 640, 620, 22, {
    size: 14,
    color: MUTED,
  });
}

function addAppendixSlideTwo(pres) {
  const slide = pres.slides.add();
  title(slide, "07 高速采样与验证", "高速电流采集系统与波形结果");
  slide.images.add({
    data: appendix2Data,
    position: { left: 48, top: 110, width: 1184, height: 516 },
    fit: "contain",
    alt: "高速电流采集系统与波形结果",
  });
  text(slide, "左侧：采集硬件与上位机  右侧：高速电流波形 / 局部放大图", 70, 640, 680, 22, {
    size: 14,
    color: MUTED,
  });
}

async function saveBlob(blob, filePath) {
  await fs.mkdir(path.dirname(filePath), { recursive: true });
  await fs.writeFile(filePath, Buffer.from(await blob.arrayBuffer()));
}

async function main() {
  await fs.mkdir(OUT_DIR, { recursive: true });
  await fs.mkdir(PREVIEW_DIR, { recursive: true });
  photoData = await fs.readFile(PHOTO_PATH);
  appendix1Data = await fs.readFile(APPENDIX_1_PATH);
  appendix2Data = await fs.readFile(APPENDIX_2_PATH);
  const pres = Presentation.create({ slideSize: { width: W, height: H } });
  await addCover(pres);
  addOverview(pres);
  addArchitecture(pres);
  addRealtime(pres);
  addModules(pres);
  addTalkTrack(pres);
  addAppendixSlideOne(pres);
  addAppendixSlideTwo(pres);

  for (let i = 0; i < pres.slides.count; i += 1) {
    const slide = pres.slides.getItem(i);
    const png = await pres.export({ slide, format: "png", scale: 1 });
    await saveBlob(png, path.join(PREVIEW_DIR, `slide-${String(i + 1).padStart(2, "0")}.png`));
  }

  const pptx = await PresentationFile.exportPptx(pres);
  await pptx.save(PPTX_PATH);
  console.log(JSON.stringify({ pptx: PPTX_PATH, slides: pres.slides.count, previewDir: PREVIEW_DIR }, null, 2));
}

main().catch((error) => {
  console.error(error.stack || error.message || String(error));
  process.exit(1);
});
