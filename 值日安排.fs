let persons = [
    "王啸"
    "刘战"
    "石岩松"
    "周成旺"
    "曹师龙"
    "许超"
]

let weeks = [
    "星期日"
    "星期一"
    "星期二"
    "星期三"
    "星期四"
    "星期五"
    "星期六"
]

let today = 4

let weeksCount = 20

let weeksRounded = 
    weeks
    |> List.mapi (fun i x -> i,x)
    |> List.partition (fun (i, _) -> i < today)
    |> fun (a, b) -> b @ a
    |> List.map snd
    
[ for weekId in 0..weeksCount-1 ->
    [ 
        for dayId in 0..6 ->
            let dayCount = weekId * 7 + dayId
            let day = System.DateTime.Today
            let day = day.AddDays(float dayCount)
            sprintf "%s | %s | %s"
                (weeksRounded.[dayCount % weeksRounded.Length])
                (day.ToShortDateString ())
                (persons.[dayCount % persons.Length]) 
    ]
]
|> List.concat
|> List.iter (printfn "%s")
