set datafile separator ","
set terminal postscript colour solid lw 1 "Helvetica" 14
set output "pow_res.csv.ps" 
set ylabel "Power"
set xlabel "Time"
plot "pow_res.csv" using 1:2 with linespoints title "VccInt", "pow_res.csv" using 1:3 with linespoints title "VccPInt", "pow_res.csv" using 1:4 with linespoints title "VccAux", "pow_res.csv" using 1:5 with linespoints title "VccPAux", "pow_res.csv" using 1:7 with linespoints title "Vcc1V5_PS", "pow_res.csv" using 1:9 with linespoints title "VccBRAM"
