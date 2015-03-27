library(methods)
library(shiny)
library(ggplot2)
library(reshape2)

server <- function(input, output) {
    cpu <- melt(read.table("/v7/graphs/v7-bench-cpu.data", header = TRUE), id=c("Title"), variable.name="VM",value.name="Time")
    mem <- melt(read.table("/v7/graphs/v7-bench-mem.data", header = TRUE), id=c("Title"), variable.name="VM",value.name="Time")

    output$cpu <- renderPlot({
        g <- cpu[grepl(input$filter,cpu$Title),]
        ggplot(data=g, aes(x=Title,y=Time,fill=VM)) + geom_bar(stat="identity", position=position_dodge())
    })

    output$mem <- renderPlot({
        g <- mem[grepl(input$filter,mem$Title),]
        ggplot(data=g, aes(x=Title,y=Time,fill=VM)) + geom_bar(stat="identity", position=position_dodge())
    })

}

ui <- fluidPage(
    titlePanel("V7 benchmarks"),
    textInput("filter", label = "Bench title regexp",
              value = ""),
    h3("cpu"),
    plotOutput("cpu"),
    h3("memory"),
    plotOutput("mem")
    )

app <- shinyApp(ui, server, uiPattern="/")
runApp(app, port=3838, host="0.0.0.0")
