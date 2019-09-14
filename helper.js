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

const posStr = (id, cx, cy) => `${'\t\t'}Vec ${id}_POS = Vec(${cx}, ${cy});`

const codeGenFunc = el => {
	const id = idGet(el)
	const cx = cxGet(el)
	const cy = cyGet(el)
	return posStr(id, cx, cy)
}

const genCodes = R.join(
	'\n',
	R.map(
		codeGenFunc,
		R.sortBy(
			idGet,
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
const outFile = fs.readFileSync(outPath, 'utf8')
const regex = /^.*\/\/ GEN_START([\s\S]*)\/\/ GEN_END/m
const newCode = `${'\t\t'}// GEN_START
${genCodes}
${'\t\t'}// GEN_END`
fs.writeFileSync(outPath, outFile.replace(regex, newCode))
