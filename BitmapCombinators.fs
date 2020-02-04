[<Struct>]module BitmapCombinators

[<Struct>]
type Pixel = {
    R : float
    G : float
    B : float
    A : float
}

type Bitmap = Pixel [,]

let inline pxFrom255px (r:^a) (g:^a) (b:^a) (a:^a) = {
    R = float r / 255.0
    G = float g / 255.0
    B = float b / 255.0
    A = float a / 255.0
}

let clamp min max v = 
    if v < min then min
    else if v > max then max
    else v

let clamp01f = clamp 0.0 1.0

let initBitmap w h init = 
    Array2D.init w h init

let createBitmap w h px = 
    initBitmap w h (fun _ _ -> px)

let loadBitmap (path:string) = 
    use bmp = new System.Drawing.Bitmap(path)
    Array2D.init bmp.Width bmp.Height (fun x y -> 
        let px = bmp.GetPixel(x,y)
        pxFrom255px px.R px.G px.B px.A)

let saveBitmap (path:string) (bmp:Bitmap) =
    use out = new System.Drawing.Bitmap (Array2D.length1 bmp,Array2D.length2 bmp)
    bmp
    |> Array2D.iteri (fun x y p ->
        let conv = (*) 255.0 >> clamp 0.0 255.0 >> int
        let c = System.Drawing.Color.FromArgb(conv p.A,conv p.R,conv p.G,conv p.B)
        out.SetPixel (x,y,c))
    out.Save path

let alphaBlend src dst = {
    R = src.R * src.A + dst.R * (1.0-src.A)
    G = src.G * src.A + dst.G * (1.0-src.A)
    B = src.B * src.A + dst.B * (1.0-src.A)
    A = src.A + (dst.A * (1.0-src.A))
}

let additiveBlend src dst = {
    R = src.R * src.A + dst.R
    G = src.G * src.A + dst.G
    B = src.B * src.A + dst.B
    A = src.A + dst.A
}

let blendBitmap (blendMode:Pixel->Pixel->Pixel) src dst =
    assert(Array2D.length1 src = Array2D.length1 dst)
    assert(Array2D.length2 src = Array2D.length2 dst)
    Array2D.init (Array2D.length1 src) (Array2D.length2 src) (fun x y ->
        blendMode src.[x,y] dst.[x,y])

let clipBitmap x y w h (src:Bitmap) =
    Array2D.init w h (fun x2 y2 ->
        src.[x + x2,y + y2])

let gaussianDistribution rho pos =
    let posx,posy = pos
    let g = 1.0 / sqrt(2.0 * 3.141592654 * rho * rho)
    g * exp (-(posx * posx + posy * posy) / (2.0 * rho * rho))

let createCore w h (f:float*float->float) =
    Array2D.init w h (fun x y -> 
        let pos =
            (float x / float (w-1) - 0.5) * 2.0,
            (float y / float (h-1) - 0.5) * 2.0
        f pos)

let convolution core bitmap = 
    let coreW,coreH = Array2D.length1 core,Array2D.length2 core
    let coreCenterX,coreCenterY = coreW / 2,coreH / 2

    Array2D.init (Array2D.length1 bitmap) (Array2D.length2 bitmap) (fun x y ->
        seq {
            for cx in 0..coreW-1 do
                for cy in 0..coreH-1 ->
                    let coreValue = core.[cx,cy]
                    let bitmapx,bitmapy =
                        clamp 0 (-1 + Array2D.length1 bitmap) (cx - coreCenterX + x),
                        clamp 0 (-1 + Array2D.length2 bitmap) (cy - coreCenterY + y)
                    let sample = bitmap.[bitmapx,bitmapy]
                    { 
                        R = sample.R * coreValue
                        G = sample.G * coreValue
                        B = sample.B * coreValue
                        A = sample.A * coreValue
                    },coreValue
        }
        |> Seq.reduce (fun (px1,c1) (px2,c2) ->
            {
                R = px1.R + px2.R
                G = px1.G + px2.G
                B = px1.B + px2.B
                A = px1.A + px2.A
            },c1 + c2)
        |> fun (px,c) -> {
            R = px.R / c
            G = px.G / c
            B = px.B / c
            A = px.A / c
        }
    )

let nearestScaling w h srcBitmap =
    initBitmap w h (fun x y ->
        let posfx,posfy = float x / ( - 1.0 + float w),float y / (-1.0 + float h)
        let possrcx,possrcy = 
            posfx * float (-1 + Array2D.length1 srcBitmap) |> int,
            posfy * float (-1 + Array2D.length2 srcBitmap) |> int
        srcBitmap.[possrcx,possrcy])
       
type Pixel = {
    R : float
    G : float
    B : float
    A : float
}

type Bitmap = Pixel [,]

let inline pxFrom255px (r:^a) (g:^a) (b:^a) (a:^a) = {
    R = float r / 255.0
    G = float g / 255.0
    B = float b / 255.0
    A = float a / 255.0
}

let clamp min max v = 
    if v < min then min
    else if v > max then max
    else v

let clamp01f = clamp 0.0 1.0

let createBitmap w h px = 
    Array2D.init w h (fun _ _ -> px)

let loadBitmap (path:string) = 
    use bmp = new System.Drawing.Bitmap(path)
    Array2D.init bmp.Width bmp.Height (fun x y -> 
        let px = bmp.GetPixel(x,y)
        pxFrom255px px.R px.G px.B px.A)

let saveBitmap (path:string) (bmp:Bitmap) =
    use out = new System.Drawing.Bitmap (Array2D.length1 bmp,Array2D.length2 bmp)
    bmp
    |> Array2D.iteri (fun x y p ->
        let conv = (*) 255.0 >> clamp 0.0 255.0 >> int
        let c = System.Drawing.Color.FromArgb(conv p.A,conv p.R,conv p.G,conv p.B)
        out.SetPixel (x,y,c))
    out.Save path

let alphaBlend src dst = {
    R = src.R * src.A + dst.R * (1.0-src.A)
    G = src.G * src.A + dst.G * (1.0-src.A)
    B = src.B * src.A + dst.B * (1.0-src.A)
    A = src.A + (dst.A * (1.0-src.A))
}

let additiveBlend src dst = {
    R = src.R * src.A + dst.R
    G = src.G * src.A + dst.G
    B = src.B * src.A + dst.B
    A = src.A + dst.A
}

let blendBitmap (blendMode:Pixel->Pixel->Pixel) src dst =
    assert(Array2D.length1 src = Array2D.length1 dst)
    assert(Array2D.length2 src = Array2D.length2 dst)
    Array2D.init (Array2D.length1 src) (Array2D.length2 src) (fun x y ->
        blendMode src.[x,y] dst.[x,y])

()
