  library(dplyr)
  
  data <- read.table("dados/3.1MB/localClientData-20-4~27.txt", header = TRUE)
  
  minBufferPower<-4
  maxBufferPower<-27
  
  newData<-c()
  bufferSizes<-c()
  
  for(i in c(minBufferPower:maxBufferPower)){
    newData <- c(newData, mean(dplyr::filter(data, bufferSize == 2**i)$downloadTime)/1000000)
    bufferSizes <- c(bufferSizes, 2**i)
  }
  
  barplot(newData, names.arg=bufferSizes, ylab="Download Time(s)", xlab="Buffer Size(Bytes)")