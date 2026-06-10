#include "analisys_raport.h"

#include <iostream>
#include <fstream>
#include <string>

// Poniższy kod został umieszczony jako osobny plik, generujący jedynie nagłówek pliku html.
// Z racji powiększenia się pliku CSS, sam plik do generowania raportu stracił możliwość czytelność kodu.
// Pomoc w tworzeniu struktury, użyto budowę od w3schools
// https://www.w3schools.com/tags/tag_section.asp
// https://www.w3schools.com/howto/howto_js_accordion.asp


void Raport::WriteHtmlHeader(std::ofstream& outFile, const std::string& title) {
	outFile << R"(<!DOCTYPE html>
<html lang="pl">
<head>
	<meta charset="utf-8">
	<meta name="viewport" content="width=1920, initial-scale=1, shrink-to-fit=no">
	<title> )" << title << R"( </title>
	<style>

:root {
	/* 
	Pomysł na kolory od autora ze strony:
	https://hookagency.com/blog/website-color-schemes-2020/
	*/
	--background1: #080a0f;
	--background2: #0a0e1b;
	--line1: #ffffff11;
	--line2: #ffffff33;

	--iceCold: #45d3ff;

	--overlay1: #ffffff08;
	--overlay2: #121b31aa;
	--overlay3: #202b47aa;
	--overlayHover: #1d2d55da;

	--panel: #0f182baf;

	--text: #e6e8ee;
	--subText: #e6e8eeb8;

	--shadow: 0 11px 33px #00000088;
	--radius1: 18px;
	--radius2: 10px;

	--font: ui-sans-serif, "Liberation Sans", Arial;
	--monoFont: ui-monospace, "Liberation Mono";
}

*{
	box-sizing: border-box;
}

html {
	height: 100%;
}

body {
	margin: 0;
	font-family: var(--font);
	color: var(--text);
	background:
		radial-gradient(1200px 800px at 18% 0%, #a0d2eb29, transparent 86%),
		radial-gradient(800px 600px at 68% 10%, #8458b32a, transparent 68%),
		linear-gradient(180deg, var(--background1), var(--background2));
	background-attachment: fixed;
	display: flex;
	flex-direction: column;
	min-height: 100vh;
}

.app {
	width: 1680px;
	margin: 0 auto;
	padding: 33px 18px;
	flex: 1;
}

.topBar {
	display: flex;
	justify-content: center;
	align-items: center;
	text-align: center;
	gap: 18px;
	padding: 18px;
	border: 1px solid var(--line1);
	border-radius: var(--radius1);
	background: linear-gradient(180deg, var(--overlay3), var(--overlay2));
	box-shadow: var(--shadow);
	line-height: 1.25;
}

.title h1{
	margin: 0 0 4px 0;
	font-size: 30px;
	color: #ffffff;
	letter-spacing: 0.25px;
}

.title .subtitle {
	margin: 0;
	font-size: 25px;
	font-weight: 500;
	color: var(--subText);
	letter-spacing: 0.2px;
}

.layout {
	margin-top: 33px;
	display: grid;
	grid-template-columns: 1fr 300px;
	gap: 33px;
}

.mainColumn {
	display: flex;
	flex-direction: column;
	gap: 33px;
}

.panel {
	padding: 18px;
	border-radius: var(--radius1);
	border: 1px solid var(--line1);
	background: var(--panel);
	box-shadow: var(--shadow);
}

.panelHead {
	display: flex;
	align-items: baseline;
	justify-content: space-between;
	gap: 18px;
	padding: 2px 2px 10px 2px;
	border-bottom: 1px solid var(--line2);
	margin-bottom: 18px;
}

.panel h2 {
	margin: 0 auto;
	font-size: 25px;
	letter-spacing: 0.25px;
}

.cards {
	display: grid;
	grid-template-columns: 1fr 1fr;
	gap: 18px;
}

.card {
	padding: 10px;
	border-radius: var(--radius2);
	border: 1px solid var(--line1);
	background: var(--overlay1);
}

.card h3{
	text-align: center;
	margin: 0 auto 10px auto;
	font-size: 18px;
	color: var(--text);
	letter-spacing: 0.15px;
}

.card table{
	margin: 0 auto;
}

.cards.ksCards > .card {
	overflow: hidden;
}

.cards.ksCards .description {
	margin: 0;
}

.cards.ksCards .tableWrap {
	max-height: 222px;
	overflow: auto;
}

.cards.ksCards table {
	width: 100%;
	margin: 0;
}

.description {
	margin: 0 0 10px 0;
	padding: 14px 18px;
	border-radius: var(--radius1);
	border: 1px solid var(--line1);
	background: #ffffff08;
	color: var(--text);
	font-size: 14px;
}

.accordion {
	border: 1px solid var(--line1);
	border-radius: var(--radius2);
	background: var(--overlay1);
	overflow: hidden;
	margin-top: 14px;
}

/* Jedynie dziecko tabeli */
.accordion > summary {
	cursor: pointer;
	display: flex;
	align-items: center;
	justify-content: center;
	gap: 18px;
	padding: 14px;
	border-bottom: 2px solid var(--line1);
	user-select: none;
	font-weight: 800;
	list-style: none;
	letter-spacing: 0.25px;
}

.accordion > summary::-webkit-details-marker {
	display: none;
}

.accordion > summary:before {
	content: "▸";
	flex: 0 0 24px 0;
	color: var(--iceCold);
	text-align: left;
	font-size: 24px;
}

.accordion > summary:after {
	content: "";
	flex: 0 0 24px 0;
}

.accordion[open] > summary:before {
	content: "▾";
}

.accordion > summary > span {
	flex: 1;
	text-align: center;
	font-size: 18px;
}

.tableWrap {
	max-height: 68vh;
	overflow: auto;
}

.tableWrap thead th{
	position: sticky;
	top: 0;
	z-index: 1;
}

table {
	width: 100%;
	border-collapse: separate;
	border-spacing: 0;
	font-size: 14px;
}

thead th{
	z-index: 1;
	background: linear-gradient(180deg, var(--overlay3), var(--overlay2));
	color: var(--text);
	text-align: center;
	padding: 10px;
	border-bottom: 1px solid var(--line1);
	white-space: nowrap;
}

#all .tableWrap thead th {
	background: linear-gradient(180deg, #202b47, #121b31);
}

tbody td{
	padding: 10px;
	border-bottom: 1px solid var(--line1);
	color: var(--text);
}

tbody tr:nth-child(odd) td {
	background: var(--overlay1);
}

tbody tr:hover td {
	background: var(--overlayHover);
}

thead th, tbody td {
	width: 20%;
	text-align: center;
}

.sideColumn .sticky {
	position: sticky;
	top: 18px;
}

.toc {
	display: flex;
	flex-direction: column;
	gap: 10px;
	margin-top: 10px;
}

.toc a {
	text-decoration: none;
	padding: 14px 14px 14px 18px;
	border: 1px solid var(--line1);
	border-radius: var(--radius2);
	background: var(--overlay1);
	color: var(--text);
}

.toc a:hover {
	background: var(--overlay2);
	border-color: var(--line2);
}

.toc a.tocMain {
	border-color: #44bbdf44;
	background: #44bbdf11;
	font-weight: 666;
	font-size: 18px;
	color: var(--text);
}

.toc a.tocMain:hover {
	border-color: #44bbdf66;
	background: #44bbdf22;
}

.divider {
	height: 2px;
	background: var(--line1);
	margin: 8px 0;
}

.footer {
	width: 1680px;
	margin: 22px auto 0 auto;
	padding: 18px;
	text-align: center;
	font-size: 14px;
	color: var(--subText);
	border-top: 1px solid var(--line2);
}

.footerContent {
	display: flex;
	justify-content: center;
	align-items: center;
	gap: 10px;
	flex-wrap: wrap;
}

.footerContent span {
	display: flex;
	align-items: center;
	gap: 10px;
}

.footer strong {
	color: var(--text);
}

a:focus-visible, summary:focus-visible {
	outline: 2px solid #44bbdf88;
	outline-offset: 2px;
	border-radius: var(--radius2);
}

/* Scrollbar */
.tableWrap::-webkit-scrollbar {
	height: 10px;
	width: 10px;
}

.tableWrap::-webkit-scrollbar-thumb {
	background: #ffffff22;
	border: 3px solid #000000;
	background-clip: padding-box;
	border-radius: 999px;
}

.tableWrap::-webkit-scrollbar-thumb:hover {
	background: #ffffff33;
}

	</style>
</head>
<body>
	<div class="app">
		<header class="topBar">
			<div class="title">
				<h1>System analizy statystycznej gier losowych</h1>
				<h1 class="subtitle">)" << title << R"(</h1>
			</div>
		</header>
		
		<main class="layout">
			<section class="mainColumn">
)"; 
}