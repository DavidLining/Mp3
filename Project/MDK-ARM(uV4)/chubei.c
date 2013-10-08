	/* �����ļ�ϵͳ */
	g_Result = f_mount(0, &g_fs);	/* Mount a logical drive */
	if (g_Result != FR_OK)
	{
		DispError(STR_OpenSDErr);
	}
	else
	{
		ViewCardDir();	/* ��ӡSD����Ŀ¼ */
		

		/* �򿪸��ļ��� */
		result = f_opendir(&DirInf, "/"); /* ���������������ӵ�ǰĿ¼��ʼ */
		if (result != FR_OK)
		{
			printf("Open Root Directory Error (%d)\r\n", result);
			DispError(STR_OpenSDErr);
		}
		fFound = 0;
		for(;;)
		{
			result = f_readdir(&DirInf,&g_FileInf); 		/* ��ȡĿ¼��������Զ����� */
			if (result != FR_OK || g_FileInf.fname[0] == 0)
			{
				break;
			}

			if (g_FileInf.fname[0] == '.')	/* ��ʾĿ¼ */
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
			/* �򿪸�Ŀ¼�µ� test.mp3 �ļ� */
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