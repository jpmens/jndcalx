marktopdf=/usr/local/bin/markdown2pdf

jndcalx.pdf: jndcalx.markdown
	$(marktopdf) -N --toc -o jndcalx.pdf jndcalx.markdown

jndcalx.markdown: jndcalx.pp macros
	filepp -w -imacros macros -o jndcalx.markdown jndcalx.pp 
