	/* 挂载文件系统 */
	g_Result = f_mount(0, &g_fs);	/* Mount a logical drive */
	if (g_Result != FR_OK)
	{
		DispError(STR_OpenSDErr);
	}
	else
	{
		ViewCardDir();	/* 打印SD卡根目录 */
		

		/* 打开根文件夹 */
		result = f_opendir(&DirInf, "/"); /* 如果不带参数，则从当前目录开始 */
		if (result != FR_OK)
		{
			printf("Open Root Directory Error (%d)\r\n", result);
			DispError(STR_OpenSDErr);
		}
		fFound = 0;
		for(;;)
		{
			result = f_readdir(&DirInf,&g_FileInf); 		/* 读取目录项，索引会自动下移 */
			if (result != FR_OK || g_FileInf.fname[0] == 0)
			{
				break;
			}

			if (g_FileInf.fname[0] == '.')	/* 表示目录 */
			{
				continue;
			}

			if (g_FileInf.fattrib != AM_DIR)
			{
				uint8_t Len;

				Len = strlen(g_FileInf.fname);
				if (Len >= 5)
				{
					if (memcmp(&g_FileInf.fname[Len - 3], "MP3", 3) == 0)
					{
						fFound = 1;
						break;
					}
				}
			}
		}

		if (fFound == 1)
		{
			/* 打开根目录下的 test.mp3 文件 */
			g_Result = f_open(&g_File, g_FileInf.fname, FA_OPEN_EXISTING | FA_READ);
			if (g_Result !=  FR_OK)
			{
				DispError(STR_OpenFileErr);
			}
		}
		else
		{
			DispError(STR_OpenFileErr);
		}
	}