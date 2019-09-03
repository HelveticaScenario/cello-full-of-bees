const fs = require('fs')
const path = require('path')
const R = require('ramda')
const identifier = process.argv[3]
const outPath = path.resolve(process.argv[4])
const svg = fs.readFileSync(path.resolve(process.argv[2]), 'utf8')

var DOMParser = require('xmldom').DOMParser
var doc = new DOMParser().parseFromString(svg, 'image/svg+xml')
const attr = R.curry((attr, el) => el.getAttribute(attr))

const idGet = attr('id')
const cxGet = attr('cx')
const cyGet = attr('cy')

const IN = 'in'
const OUT = 'out'
const PARAM = 'param'
const LIGHT = 'light'
const WIDGET = 'widget'

const inStr = (id, cx, cy) => `${'\t\t'}auto ${id}_POS = Vec(${cx}, ${cy});`

const outStr = (id, cx, cy) => `${'\t\t'}auto ${id}_POS = Vec(${cx}, ${cy});`

const paramStr = (id, cx, cy) => `${'\t\t'}auto ${id}_POS = Vec(${cx}, ${cy});`

const lightStr = (id, cx, cy) => `${'\t\t'}auto ${id}_POS = Vec(${cx}, ${cy});`

const widgetStr = (id, cx, cy) => `${'\t\t'}auto ${id}_POS = Vec(${cx}, ${cy});`

const groupFunc = el => {
	const id = idGet(el).toLowerCase()
	if (id.endsWith('_in')) {
		return IN
	}

	if (id.endsWith('_out')) {
		return OUT
	}

	if (id.endsWith('_param')) {
		return PARAM
	}

	if (id.endsWith('_light')) {
		return LIGHT
	}

	return WIDGET
}

const codeGenFunc = R.curry((key, el) => {
	const id = idGet(el)
	const cx = cxGet(el)
	const cy = cyGet(el)
	switch (key) {
		case IN: {
			return inStr(id, cx, cy)
		}
		case OUT: {
			return outStr(id, cx, cy)
		}
		case PARAM: {
			return paramStr(id, cx, cy)
		}
		case LIGHT: {
			return lightStr(id, cx, cy)
		}
		case WIDGET: {
			return widgetStr(id, cx, cy)
		}
	}
})

const genCodes = R.join(
	'\n',
	R.flatten(
		R.values(
			R.mapObjIndexed(
				(val, key) => R.map(codeGenFunc(key), val),
				R.mapObjIndexed(
					R.sortBy(
						R.compose(
							parseInt,
							cyGet
						)
					),
					R.groupBy(
						groupFunc,
						R.filter(
							R.compose(
								R.not,
								R.isEmpty,
								idGet
							),
							Array.from(doc.getElementsByTagName('circle'))
						)
					)
				)
			)
		)
	)
)
const outFile = fs.readFileSync(outPath, 'utf8')
const regex = /\/\/ GEN_START([\s\S]*)\/\/ GEN_END/
const newCode = `// GEN_START
${genCodes}
// GEN_END`
fs.writeFileSync(outPath, outFile.replace(regex, newCode))
