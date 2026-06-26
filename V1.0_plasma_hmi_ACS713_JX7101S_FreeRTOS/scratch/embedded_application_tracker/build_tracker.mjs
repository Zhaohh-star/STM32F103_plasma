import fs from "node:fs/promises";
import { SpreadsheetFile, Workbook } from "@oai/artifact-tool";

const outputDir = "D:/Program Files/STM32CubeIDE/workspace_1.19.0/V1.0_plasma_hmi_ACS713_JX7101S_FreeRTOS/outputs/embedded_application_tracker";
const outputPath = `${outputDir}/嵌入式投递跟踪表_淡绿色.xlsx`;

const workbook = Workbook.create();

const statuses = ["待投递", "已投递", "笔试/测评", "一面", "二面/终面", "HR面", "Offer", "已拒绝", "已结束"];
const priorities = ["高", "中", "低"];
const matchLevels = ["高匹配", "较匹配", "一般", "不匹配"];
const channels = ["Boss直聘", "猎聘", "前程无忧", "智联招聘", "官网", "校招/就业网", "内推", "公众号", "邮件", "其他"];
const resumeVersions = ["嵌入式通用版", "医疗器械版", "制造业版", "BLE/物联网版", "实习版", "待修改"];
const softGreen = "#E1F9CA";
const softGreenAccent = "#F3FCEB";
const darkGreenText = "#214121";

function setWidths(sheet, widths) {
  widths.forEach((width, idx) => {
    const col = String.fromCharCode(65 + idx);
    sheet.getRange(`${col}:${col}`).format.columnWidthPx = width;
  });
}

function addValidation(sheet, range, values) {
  sheet.getRange(range).dataValidation = {
    rule: { type: "list", values },
  };
}

function styleSheet(sheet, titleRange, tableRange, tableName, widths, statusCol, priorityCol, matchCol, channelCol, resumeCol) {
  sheet.showGridLines = false;
  sheet.mergeCells(titleRange);
  sheet.getRange(titleRange.split(":")[0]).format = {
    fill: softGreen,
    font: { bold: true, color: darkGreenText, size: 16 },
  };
  sheet.getRange(titleRange).format.rowHeightPx = 34;

  sheet.getRange("A3:H4").format = {
    fill: softGreenAccent,
    font: { bold: true, color: darkGreenText },
  };
  sheet.getRange("A3:H4").format.borders = {
    insideHorizontal: { style: "Continuous", color: "#C9D6E2" },
    insideVertical: { style: "Continuous", color: "#C9D6E2" },
  };

  sheet.getRange(tableRange.split(":")[0] + ":" + tableRange.split(":")[1].replace(/\d+$/, "7")).format = {
    fill: softGreen,
    font: { bold: true, color: darkGreenText },
  };
  sheet.getRange(tableRange).format.wrapText = true;
  sheet.getRange(tableRange).format.borders = {
    insideHorizontal: { style: "Continuous", color: "#E5E7EB" },
    insideVertical: { style: "Continuous", color: "#E5E7EB" },
  };
  sheet.freezePanes.freezeRows(7);
  setWidths(sheet, widths);

  const table = sheet.tables.add(tableRange, true, tableName);
  table.style = "TableStyleLight1";
  table.showFilterButton = true;
  table.getHeaderRowRange().format = {
    fill: softGreen,
    font: { bold: true, color: darkGreenText },
  };

  addValidation(sheet, `${statusCol}8:${statusCol}107`, statuses);
  addValidation(sheet, `${priorityCol}8:${priorityCol}107`, priorities);
  addValidation(sheet, `${matchCol}8:${matchCol}107`, matchLevels);
  addValidation(sheet, `${channelCol}8:${channelCol}107`, channels);
  addValidation(sheet, `${resumeCol}8:${resumeCol}107`, resumeVersions);

  sheet.getRange("H8:I107").setNumberFormat("yyyy-mm-dd");
  sheet.getRange("V8:V107").setNumberFormat("yyyy-mm-dd");
}

function buildInternshipSheet() {
  const sheet = workbook.worksheets.add("实习投递表");
  sheet.getRange("A1:W1").values = [["嵌入式实习投递表"]];
  sheet.getRange("A3:H3").values = [["总投递", "待投递", "已投递", "面试中", "Offer", "已拒绝", "本周需跟进", "高匹配"]];
  sheet.getRange("A4:H4").formulas = [[
    '=COUNTA(B8:B107)',
    '=COUNTIF(J8:J107,"待投递")',
    '=COUNTIF(J8:J107,"已投递")',
    '=COUNTIF(J8:J107,"一面")+COUNTIF(J8:J107,"二面/终面")+COUNTIF(J8:J107,"HR面")',
    '=COUNTIF(J8:J107,"Offer")',
    '=COUNTIF(J8:J107,"已拒绝")',
    '=COUNTIFS(V8:V107,">="&TODAY(),V8:V107,"<="&TODAY()+7,J8:J107,"<>已结束")',
    '=COUNTIF(K8:K107,"高匹配")',
  ]];

  const headers = [
    "序号", "公司名称", "行业/公司类型", "城市", "岗位名称", "岗位方向",
    "投递渠道", "投递日期", "可到岗时间", "投递状态", "匹配度", "优先级",
    "技术关键词", "简历版本", "JD链接", "内推/联系人", "联系方式",
    "笔试/测评", "一面时间", "二面/终面", "结果/反馈", "下次跟进日期", "备注/复盘"
  ];
  sheet.getRange("A7:W7").values = [headers];
  sheet.getRange("A8:A107").formulas = Array.from({ length: 100 }, (_, i) => [`=IF(B${8 + i}="","",ROW()-7)`]);

  styleSheet(
    sheet,
    "A1:W1",
    "A7:W107",
    "InternshipApplications",
    [50, 150, 130, 80, 160, 150, 110, 100, 110, 105, 90, 80, 220, 130, 220, 120, 140, 110, 120, 120, 160, 120, 220],
    "J", "L", "K", "G", "N"
  );
  return sheet;
}

function buildJobSheet() {
  const sheet = workbook.worksheets.add("找工作的投递表");
  sheet.getRange("A1:Y1").values = [["嵌入式正式工作投递表"]];
  sheet.getRange("A3:H3").values = [["总投递", "待投递", "已投递", "面试中", "Offer", "已拒绝", "本周需跟进", "高优先级"]];
  sheet.getRange("A4:H4").formulas = [[
    '=COUNTA(B8:B107)',
    '=COUNTIF(K8:K107,"待投递")',
    '=COUNTIF(K8:K107,"已投递")',
    '=COUNTIF(K8:K107,"一面")+COUNTIF(K8:K107,"二面/终面")+COUNTIF(K8:K107,"HR面")',
    '=COUNTIF(K8:K107,"Offer")',
    '=COUNTIF(K8:K107,"已拒绝")',
    '=COUNTIFS(X8:X107,">="&TODAY(),X8:X107,"<="&TODAY()+7,K8:K107,"<>已结束")',
    '=COUNTIF(M8:M107,"高")',
  ]];

  const headers = [
    "序号", "公司名称", "行业/公司类型", "城市/Base", "岗位名称", "岗位方向",
    "岗位级别", "薪资范围", "投递渠道", "投递日期", "投递状态", "匹配度",
    "优先级", "技术关键词", "简历版本", "JD链接", "内推/联系人", "联系方式",
    "笔试/测评", "一面时间", "二面/终面", "HR/谈薪", "结果/反馈",
    "下次跟进日期", "备注/复盘"
  ];
  sheet.getRange("A7:Y7").values = [headers];
  sheet.getRange("A8:A107").formulas = Array.from({ length: 100 }, (_, i) => [`=IF(B${8 + i}="","",ROW()-7)`]);

  styleSheet(
    sheet,
    "A1:Y1",
    "A7:Y107",
    "FulltimeApplications",
    [50, 150, 130, 90, 160, 150, 95, 100, 110, 100, 105, 90, 80, 220, 130, 220, 120, 140, 110, 120, 120, 110, 160, 120, 220],
    "K", "M", "L", "I", "O"
  );
  return sheet;
}

buildInternshipSheet();
buildJobSheet();

for (const sheetName of ["实习投递表", "找工作的投递表"]) {
  const sheet = workbook.worksheets.getItem(sheetName);
  sheet.getRange("A6").values = [["投递明细"]];
  sheet.getRange("A6").format = { font: { bold: true, color: darkGreenText } };
}

const check1 = await workbook.inspect({
  kind: "table",
  range: "实习投递表!A1:W12",
  include: "values,formulas",
  tableMaxRows: 12,
  tableMaxCols: 23,
});
console.log(check1.ndjson);

const check2 = await workbook.inspect({
  kind: "table",
  range: "找工作的投递表!A1:Y12",
  include: "values,formulas",
  tableMaxRows: 12,
  tableMaxCols: 25,
});
console.log(check2.ndjson);

const errors = await workbook.inspect({
  kind: "match",
  searchTerm: "#REF!|#DIV/0!|#VALUE!|#NAME\\?|#N/A",
  options: { useRegex: true, maxResults: 100 },
  summary: "final formula error scan",
});
console.log(errors.ndjson);

await workbook.render({ sheetName: "实习投递表", range: "A1:W18", scale: 1, format: "png" });
await workbook.render({ sheetName: "找工作的投递表", range: "A1:Y18", scale: 1, format: "png" });

await fs.mkdir(outputDir, { recursive: true });
const output = await SpreadsheetFile.exportXlsx(workbook);
await output.save(outputPath);
console.log(outputPath);
