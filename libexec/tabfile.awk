function word() {
	w=""
	end=0
	if (substr($0, 1, 1) == "\"") {
		s=substr($0, 2)
		if (match(s, /"[\t ]/)) {
			end=RSTART+RLENGTH+1
			w=substr(s, 1, RSTART+RLENGTH-3)
		}
	} else {
		if (match($0, /[^\t ]*[\t ]/)) {
			end=RSTART+RLENGTH
			w=substr($0, RSTART, RLENGTH-1)
		}
	}
	$0=substr($0, end)
	return sprintf("%d:%s", length(w), w)
}

/^#/ || /^$/ { next }
{
	for (i=0; i<4; i++)
		printf "%s,",word()
	printf "%d:%s,\n",length($0),$0
}
