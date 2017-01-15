#!/usr/bin/Rscript

# Take any number of files or directories as arguments.
# Any file should have only one column, containing the relevant data.
# File name is the title of the plot.
# Plot both timeseries and reverse cdf on a log-log plot.

# Stuff like axes labels should in a configuration file, perhaps with a
# hardcoded name.

library(docopt)

"Usage: DistributionPlot.r [-ho FILE] [--rcdf] [INPUT ...]

-h --help                  show this
-o FILE, --output FILE     specify output file [default: ./Rplots.pdf]
-r --rcdf                  generate rcdfs instead of time series" -> doc

options = docopt(doc)

for (f in options$INPUT) {
    print(f);
}
