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
-o FILE, --output FILE     specify output file [default: ./DPlots.pdf]
-r --rcdf                  generate rcdfs instead of time series" -> doc



addToPlot <- function(filename) {
    data = read.table(filename)
    plot(data[,1], 1-data[,2], main=tools::file_path_sans_ext(basename(filename)),
         type="l", log="xy")
}
main <- function() {
    options = docopt(doc)

    pdf(options$output);
    for (f in options$INPUT) {
        if (dir.exists(f)) {
            for (sf in list.files(f, full.names=TRUE)) {
                if (!dir.exists(sf)) {
                    addToPlot(sf)
                }
            }
        }
        else if (file.exists(f)) {
            addToPlot(f)
        } # Otherwise, we ignore
    }
    dev.off();
}
main()
