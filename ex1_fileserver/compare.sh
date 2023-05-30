rm -f *.diff
for i in $*
do
#cp $i $i.test
	srci=($(ls $i/*))
	srci=${srci[0]}
	echo $srci
	for j in $*
	do
		srcj=($(ls $j/*))
		srcj=${srcj[0]}
		if [ $i \< $j ]
		then
			echo $srcj
			if [[ $srci =~ ".cpp" ]] && [[ $srcj =~ ".cpp" ]]
			then
				diff --strip-trailing-cr $srci $srcj > $i.$j.diff
			elif [[ $srci =~ ".c" ]] && [[ $srcj =~ ".c" ]]
			then
				diff --strip-trailing-cr $srci $srcj > $i.$j.diff
			elif [[ $srci =~ ".java" ]] && [[ $srcj =~ ".java" ]]
			then
				diff --strip-trailing-cr $srci $srcj > $i.$j.diff
			elif [[ $srci =~ ".py" ]] && [[ $srcj =~ ".py" ]]
			then
				diff --strip-trailing-cr $srci $srcj > $i.$j.diff
			else
				echo "not the same" $i/* $j/*
			fi
		fi
	done
done
