  library(dplyr)
  
  data <- read.table("dados/3.2MB/remoteClientData-20-4~27.txt", header = TRUE)
  
  minBufferPower<-14
  maxBufferPower<-27
  
  newData<-c()
  bufferSizes<-c()
  
  for(i in c(minBufferPower:maxBufferPower)){
    newData <- c(newData, mean(dplyr::filter(data, bufferSize == 2**i)$downloadTime))
    bufferSizes <- c(bufferSizes, 2**i)
  }
  
  barplot(newData, names.arg=bufferSizes, ylab="Download Time", xlab="Buffer Size")
  
  
  #mean(dplyr::filter(data, bufferSize == 1)$downloadTime)
  
  #for(dataNode in data)
  
  #barplot(data$downloadTime, names.arg=data$bufferSize, ylab="Download Time", xlab="Buffer Size")