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
