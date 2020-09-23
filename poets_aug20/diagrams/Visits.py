from jdiag import *

from com.xrbpowered.jdiagram.data import ExpFormatter

from com.xrbpowered.jdiagram.chart import Page
from com.xrbpowered.jdiagram.chart import ScatterChart
from com.xrbpowered.jdiagram.chart.ScatterChart import Population
from com.xrbpowered.jdiagram.chart import Anchor

from com.xrbpowered.jdiagram.chart import StepRenderer

resNames = [
	('result_g%d_AH', 'async, no weights, %d-con, bucket-fill', 'stroke:#5b0'),
	('result_g%d_AW', 'async, weights, %d-con, bucket-fill', 'stroke:#009'),
	('metis_g%d_AW', 'async, weights, %d-con, metis', 'stroke:#e90')
]
cons = [
	(4, 'stroke-width:1.5;stroke-dasharray: 2 2'),
	(8, 'stroke-width:1.5')
]

data = Data.read(File('count_visits.csv'))

page = Page(1).setGap(5)

for log in [False, True]:
	# set up a new chart
	chart = ScatterChart().setTitle('Histogram of Visits Distribution').setSize(500, 300)
	chart.setMargins(60, 20, 40, 100)
	chart.legend.setCols(2).posBottom(-40).setItemSize(220, 20)
	chart.clipChart = True

	# set up axes
	chart.axisx.setRange(0, 250, 50) \
			.setAnchor(Anchor.bottom).setLabel('visits').setNumberFmt('%.0f')
	if not log:
		chart.axisy.setRange(False, 0, 20000, 5000) \
				.setAnchor(Anchor.left).setLabel('nodes', Anchor.left.offset(-45)).setNumberFmt('%.0f')
	else:
		chart.axisy.setRange(True, 1, 65536, 4) \
				.setAnchor(Anchor.left).setLabel('nodes', Anchor.left.offset(-30)).setNumberFormatter(ExpFormatter.svgExp(2, 5))

	# add data lines
	for (res, resLabel, resStyle) in resNames:
		for (con, conStyle) in cons:
			phdr = res % con
			chart.addPopLegend(resLabel % con,
				Population(data, 'visits', phdr, 'fill:none;%s;%s' % (resStyle, conStyle)).setRenderer(StepRenderer())
			)
	page.add(chart)

# finish and print
page.printPage(System.out)
