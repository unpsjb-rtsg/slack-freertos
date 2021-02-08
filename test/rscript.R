library(plyr)
library(stringr)    # str_extract
library(reshape2)   # melt
library(ggplot2)    # ggplot

source('summarySE.R')  # SummarySE function

# read the results
r <- read.table("tests/Test1/results.txt")

# extract UF and number of tasks from the filename
r$u <- str_extract(r$V12, "u[0-9]*")
r$n <- str_extract(r$V12, "n[0-9]*")

# task id
r$task <- paste("Task", r$V1)

# no longer need V1 and V12 columns
r$V1 <- NULL
r$V12 <- NULL

# change data frame colnames
colnames(r) <- c(1:10,"slack","method","calc","u","n","task")

# melt for easier plotting
rm <- melt(r, id.vars=c("n","u","task","slack","method","calc"))

# summary -- per UF
uf_summary <- summarySE(rm, "value", groupvars=c("u","method"))

# summary -- per task and UF
task_summary <- summarySE(rm, "value", groupvars=c("u", "method", "task"))

uf_plot <- ggplot(uf_summary, aes(x=u, y=value, group=method, colour=method)) +     
  geom_errorbar(aes(ymin=value-se, ymax=value+se), colour="black", width=.1) +
  geom_line() + geom_point(size=4, shape=21, fill="white") +
  scale_colour_hue(name="Method", l=30) +
  xlab("U.F.") +  
  ylab("CPU cycles") +
  expand_limits(y=0) +
  ggtitle("CPU cycles mean cost per U.F.") +    
  theme_bw()

task_plot <- ggplot(task_summary, aes(x=u, y=value, group=task, colour=task)) +     
  geom_errorbar(aes(ymin=value-se, ymax=value+se), colour="black", width=.1) +
  geom_line() + geom_point(size=3, shape=21, fill="white") +
  scale_colour_hue(name="Task", l=30) +
  xlab("U.F.") +
  ylab("CPU cycles") +   
  expand_limits(y=0) +
  ggtitle("CPU cycles mean cost per U.F.") +    
  theme_bw()  

method_task_plot_list <- task_plot_list <- function(data) {
  l <- list()
  for (x in unique(data$method)) {
    
    ts <- summarySE(data[data$method == x,], "value", groupvars=c("u", "task"))
    
    l[[x]] <-
      ggplot(ts, aes(x=u, y=value, group=task, colour=task)) +     
      geom_errorbar(aes(ymin=value-se, ymax=value+se), colour="black", width=.1) +
      geom_line() + geom_point(size=3, shape=21, fill="white") +
      scale_colour_hue(name="Task", l=30) +
      xlab("U.F.") +
      ylab("CPU cycles") +   
      expand_limits(y=0) +
      ggtitle(paste("CPU cycles mean cost for", x)) +    
      theme_bw()
    
  }
  l
}

task_plot_list <- function(data) {
  library(scales) # trans_breaks y trans_format
  
  l <- list()
  for (x in unique(data$task)) {
    
    ts <- summarySE(data[data$task == x,], "value", groupvars=c("u", "method"))
    
    l[[x]] <-
      ggplot(ts, aes(x=u, y=value, group=method, colour=method)) +     
      geom_errorbar(aes(ymin=value-se, ymax=value+se), colour="black", width=.1) +
      geom_line() + geom_point(size=3, shape=21, fill="white") +
      scale_colour_hue(name="Method", l=30) +
      xlab("U.F.") +
      ylab("CPU cycles") +
      scale_y_log10(breaks = trans_breaks("log10", function(x) 10^x),
                    labels = trans_format("log10", math_format(10^.x))) +
      #expand_limits(y=c(0,ceiling(max(task_summary$value)))) +
      expand_limits(y=0) +
      ggtitle(paste("CPU cycles mean cost for", x)) +    
      theme_bw()
  }
  l
}

generate_pdf <- function(file) {
  pdf(file)
  print(uf_plot)
  
  for (plot in method_task_plot_list(rm)) {
    print(plot)
  }
  
  for (plot in task_plot_list(rm)) {
    print(plot)
  }  
  dev.off()
}