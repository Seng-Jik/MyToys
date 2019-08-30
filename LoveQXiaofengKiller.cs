using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace LoveQXiaofengKiller
{
    class Program
    {
        static bool IsLoveQ(FileInfo file)
        {
            byte[] bytes = new byte[2297956];
            using (var f = file.OpenRead())
            {
                f.Read(bytes, 0, bytes.Length);
            }
            string str = System.Text.Encoding.Default.GetString(bytes);
            return str.Contains("LoveQ-BYxiaofeng_sp5");
        }

        static void CleanDir(DirectoryInfo dir)
        {
            Console.Write("扫描：");
            Console.WriteLine(dir.FullName);

            foreach(var file in dir.GetFiles())
            {
                if (file.FullName.ToLower().EndsWith(".exe"))
                {
                    if (IsLoveQ(file))
                    {
                        file.Delete();
                        Console.WriteLine("已删除病毒文件：" + file.FullName);

                        //尝试恢复文件夹
                        string hiddenDirPath = file.FullName.Substring(0, file.FullName.Length - 4);
                        var hiddenDir = new DirectoryInfo(hiddenDirPath);
                        if(hiddenDir.Exists)
                        {
                            try
                            {
                                hiddenDir.Attributes = FileAttributes.Normal;

                                Console.WriteLine("已恢复文件夹：" + hiddenDirPath);
                            }
                            catch(Exception e)
                            {
                                Console.WriteLine("文件夹恢复失败：" + hiddenDirPath);
                                Console.WriteLine("原因：" + e.Message);
                            }
                        }
                        
                    }
                }
            }
        }

        static void Main(string[] args)
        {
            Console.WriteLine("LoveQ病毒专杀工具");
            Console.WriteLine();

            foreach(var drv in DriveInfo.GetDrives())
            {
                if(drv.IsReady)
                    CleanDir(drv.RootDirectory);
            }

            Console.WriteLine("已结束，按任意键退出。");
            Console.ReadKey();
        }
    }
}
