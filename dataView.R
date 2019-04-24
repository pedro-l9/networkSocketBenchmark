  library(dplyr)
  
  data <- read.table("clientData.txt", header = TRUE)
  
  minBufferPower<-6
  maxBufferPower<-22
  
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