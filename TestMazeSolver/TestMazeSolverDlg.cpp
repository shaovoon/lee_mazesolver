
// TestMazeSolverDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "TestMazeSolver.h"
#include "TestMazeSolverDlg.h"
#include "afxdialogex.h"
#include "minicsv.h"
#include <algorithm>
#include <vector>

D2D_COLOR_F const COLOR_WHITE = { 1.0f,  1.0f,  1.0f,  1.0f };
D2D_COLOR_F const COLOR_BLACK = { 0.0f,  0.0f,  0.0f,  1.0f };
D2D_COLOR_F const COLOR_YELLOW = { 0.99f, 0.85f, 0.0f,  1.0f };
D2D_COLOR_F const COLOR_GREEN = { 0.0f,  1.0f,  0.0f,  1.0f };
D2D_COLOR_F const COLOR_RED = { 1.0f,  0.0f,  0.0f,  1.0f };

const int SHIFT_LEFT = 160;
const int OBSTACLE = 255;
const int NO_OBSTACLE = 254;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTestMazeSolverDlg dialog



CTestMazeSolverDlg::CTestMazeSolverDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TESTMAZESOLVER_DIALOG, pParent)
	, m_nWindowTimer(0)
	, m_FacingDirection(FacingDirection::North)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTestMazeSolverDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RDO_REMOVE_OBS, m_rdoObstacleRemove);
	DDX_Control(pDX, IDC_RDO_PUT_OBS, m_rdoObstaclePut);
	DDX_Control(pDX, IDC_BTN_START, m_btnStart);
	DDX_Control(pDX, IDC_BTN_STOP, m_btnStop);
	DDX_Control(pDX, IDC_BTN_CLEAR_ALL, m_btnClearAll);
}

BEGIN_MESSAGE_MAP(CTestMazeSolverDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_COMMAND(IDC_RDO_REMOVE_OBS, &CTestMazeSolverDlg::OnRdoRemoveObs)
	ON_COMMAND(IDC_RDO_PUT_OBS, &CTestMazeSolverDlg::OnRdoPutObs)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(IDC_BTN_LOAD_MAZE, &CTestMazeSolverDlg::OnBnClickedBtnLoadMaze)
	ON_BN_CLICKED(IDC_BTN_SAVE_MAZE, &CTestMazeSolverDlg::OnBnClickedBtnSaveMaze)
	ON_BN_CLICKED(IDC_BTN_START, &CTestMazeSolverDlg::OnBnClickedBtnStart)
	ON_BN_CLICKED(IDC_BTN_STOP, &CTestMazeSolverDlg::OnBnClickedBtnStop)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_SAVE_WEIGHTAGE, &CTestMazeSolverDlg::OnBnClickedBtnSaveWeightage)
	ON_BN_CLICKED(IDC_BTN_CLEAR_ALL, &CTestMazeSolverDlg::OnBnClickedBtnClearAll)
END_MESSAGE_MAP()


// CTestMazeSolverDlg message handlers

BOOL CTestMazeSolverDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_btnStart.EnableWindow(TRUE);
	m_btnStop.EnableWindow(FALSE);

	m_FacingDirection = FacingDirection::North;
	m_CurrCell.x = 0;
	m_CurrCell.y = 15;

	m_ObsMode = ObsMode::Put;
	m_SolverMode = SolverMode::Solving;
	m_rdoObstaclePut.SetCheck(BST_CHECKED);
	InitMaps();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTestMazeSolverDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		//CDialogEx::OnPaint();
		CPaintDC dc(this);
		if (!m_DCTarget)
		{
			CreateDCTarget(&dc);
		}

		ComPtr<ID2D1Bitmap> bitmap;
		m_BmpTarget->GetBitmap(bitmap.GetAddressOf());

		CRect rectClient;
		GetClientRect(&rectClient);

		RECT rect;

		rect.top = 0;
		rect.bottom = rectClient.Height();
		rect.left = SHIFT_LEFT;
		rect.right = rectClient.Width();

		m_BmpTarget->BeginDraw();
		DrawMap();
		DrawObstacles();
		m_BmpTarget->EndDraw();


		m_DCTarget->BindDC(dc.GetSafeHdc(), &rect);

		m_DCTarget->BeginDraw();

		m_DCTarget->DrawBitmap(bitmap.Get());

		if (D2DERR_RECREATE_TARGET == m_DCTarget->EndDraw())
		{
			m_DCTarget.Reset();
			Invalidate();
			return;
		}


	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTestMazeSolverDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CTestMazeSolverDlg::CreateDeviceResources(ID2D1RenderTarget* target, ComPtr<ID2D1SolidColorBrush>& brush, D2D1_COLOR_F color)
{
	HR(target->CreateSolidColorBrush(color,
		brush.ReleaseAndGetAddressOf()));
}

void CTestMazeSolverDlg::CreateDCTarget(CDC* pDC)
{
	CRect rectClient;
	GetClientRect(&rectClient);

	RECT rect;

	rect.top = 0;
	rect.bottom = rectClient.Height();
	rect.left = 0;
	rect.right = rectClient.Width();

	auto size = SizeU(rect.right,
		rect.bottom);

	auto sizeF = SizeF(rect.right,
		rect.bottom);

	// Create a pixel format and initial its format
	// and alphaMode fields.
	// https://docs.microsoft.com/en-gb/windows/win32/direct2d/supported-pixel-formats-and-alpha-modes#supported-formats-for-id2d1devicecontext
	D2D1_PIXEL_FORMAT pixelFormat = D2D1::PixelFormat(
		DXGI_FORMAT_B8G8R8A8_UNORM,
		D2D1_ALPHA_MODE_PREMULTIPLIED
	);

	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties();
	props.pixelFormat = pixelFormat;

	HR(FactorySingleton::GetGraphicsFactory()->CreateDCRenderTarget(&props,
		m_DCTarget.ReleaseAndGetAddressOf()));

	m_DCTarget->BindDC(pDC->GetSafeHdc(), &rect);

	HR(m_DCTarget->CreateCompatibleRenderTarget(m_BmpTarget.ReleaseAndGetAddressOf()));

	m_BmpTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
	m_BmpTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);

	CreateDeviceResources(m_BmpTarget.Get(), m_BmpBlackBrush, COLOR_BLACK);
	CreateDeviceResources(m_BmpTarget.Get(), m_BmpWhiteBrush, COLOR_WHITE);
	CreateDeviceResources(m_BmpTarget.Get(), m_BmpYellowBrush, COLOR_YELLOW);
	CreateDeviceResources(m_BmpTarget.Get(), m_BmpGreenBrush, COLOR_GREEN);
	CreateDeviceResources(m_BmpTarget.Get(), m_BmpRedBrush, COLOR_RED);
}

void CTestMazeSolverDlg::DrawMap()
{
	if (!m_BmpTarget)
		return;

	CRect rectClient;
	GetClientRect(&rectClient);

	auto rect = RectF(rectClient.left, rectClient.top, rectClient.right, rectClient.bottom);

	float cell_dim = rectClient.Height() / 16.0f;
	float sum = 0.0f;
	m_BmpTarget->FillRectangle(&rect, m_BmpBlackBrush.Get());
	for (int i = 0; i <= 16; ++i) // vertical lines
	{
		D2D1_POINT_2F p0{ sum, 0.0f };
		D2D1_POINT_2F p1{ sum, rectClient.Height() };
		m_BmpTarget->DrawLine(p0, p1, m_BmpWhiteBrush.Get());
		sum += cell_dim;
	}
	sum = 0.0f;
	for (int i = 0; i <= 16; ++i) // horizontal lines
	{
		D2D1_POINT_2F p0{ 0.0f, sum };
		D2D1_POINT_2F p1{ rectClient.Height(), sum };
		m_BmpTarget->DrawLine(p0, p1, m_BmpWhiteBrush.Get());
		sum += cell_dim;
	}
	// draw yellow target
	auto rectTarget = RectF(cell_dim * 12 + 1.0f, cell_dim * 3 + 1.0f, cell_dim * 13 - 1.0f, cell_dim * 4 - 1.0f);
	m_BmpTarget->FillRectangle(&rectTarget, m_BmpYellowBrush.Get());
	auto rectStart = RectF(cell_dim * 0 + 1.0f, cell_dim * 15 + 1.0f, cell_dim * 1 - 1.0f, cell_dim * 16 - 1.0f);
	m_BmpTarget->FillRectangle(&rectStart, m_BmpGreenBrush.Get());

	D2D1_ELLIPSE ell;
	ell.point = Point2F(m_CurrCell.x * cell_dim + (cell_dim / 2.0f), m_CurrCell.y * cell_dim + (cell_dim / 2.0f));
	ell.radiusX = (cell_dim / 2.0f) - 3.0f;
	ell.radiusY = (cell_dim / 2.0f) - 3.0f;
	if (m_SolverMode == SolverMode::Solving)
		m_BmpTarget->FillEllipse(ell, m_BmpGreenBrush.Get());
	else
		m_BmpTarget->FillEllipse(ell, m_BmpRedBrush.Get());
}

void CTestMazeSolverDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	CClientDC dc(this);
	CreateDCTarget(&dc);
	Invalidate(FALSE);
}

void CTestMazeSolverDlg::OnRdoRemoveObs()
{
	if (m_ObsMode != ObsMode::Invalid)
		m_ObsMode = ObsMode::Remove;
}

void CTestMazeSolverDlg::OnRdoPutObs()
{
	if (m_ObsMode != ObsMode::Invalid)
		m_ObsMode = ObsMode::Put;
}

void CTestMazeSolverDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if ((nFlags & MK_LBUTTON) == 0)
	{
		CDialogEx::OnMouseMove(nFlags, point);
		return;
	}

	CRect rectClient;
	GetClientRect(&rectClient);

	int cell_dim = rectClient.Height() / 16;

	int x = (point.x - SHIFT_LEFT) / cell_dim;
	int y = point.y / cell_dim;
	if (x < 0 || x > 15 || y < 0 || y > 15)
	{
		CDialogEx::OnMouseMove(nFlags, point);
		return;
	}

	if ((x == 12 && y == 3) || (x == 0 && y == 15))
	{
		CDialogEx::OnMouseMove(nFlags, point);
		return;
	}

	if (m_ObsMode == ObsMode::Put)
	{
		if (m_ObsMap[x][y] != OBSTACLE)
		{
			m_ObsMap[x][y] = OBSTACLE;
			Invalidate(FALSE);
		}
	}
	else if (m_ObsMode == ObsMode::Remove)
	{
		if (m_ObsMap[x][y] != NO_OBSTACLE)
		{
			m_ObsMap[x][y] = NO_OBSTACLE;
			Invalidate(FALSE);
		}
	}

	CDialogEx::OnMouseMove(nFlags, point);
}

void CTestMazeSolverDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect rectClient;
	GetClientRect(&rectClient);

	int cell_dim = rectClient.Height() / 16;

	int x = (point.x - SHIFT_LEFT) / cell_dim;
	int y = point.y / cell_dim;
	if (x < 0 || x > 15 || y < 0 || y > 15)
	{
		CDialogEx::OnMouseMove(nFlags, point);
		return;
	}

	if ((x == 12 && y == 3) || (x == 0 && y == 15))
	{
		CDialogEx::OnMouseMove(nFlags, point);
		return;
	}

	if (m_ObsMode == ObsMode::Put)
	{
		if (m_ObsMap[x][y] != OBSTACLE)
		{
			m_ObsMap[x][y] = OBSTACLE;
			Invalidate(FALSE);
		}
	}
	else if (m_ObsMode == ObsMode::Remove)
	{
		if (m_ObsMap[x][y] != NO_OBSTACLE)
		{
			m_ObsMap[x][y] = NO_OBSTACLE;
			Invalidate(FALSE);
		}
	}

	CDialogEx::OnLButtonDown(nFlags, point);
}

void CTestMazeSolverDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	CDialogEx::OnLButtonUp(nFlags, point);
}

void CTestMazeSolverDlg::InitMaps()
{
	for (int y = 0; y < 16; ++y)
	{
		for (int x = 0; x < 16; ++x)
		{
			m_Map[x][y] = NO_OBSTACLE;
			m_ObsMap[x][y] = NO_OBSTACLE;
		}
	}
	m_Map[12][3] = 0;
	m_ObsMap[12][3] = 0;
}

void CTestMazeSolverDlg::InitMap()
{
	for (int y = 0; y < 16; ++y)
	{
		for (int x = 0; x < 16; ++x)
		{
			m_Map[x][y] = NO_OBSTACLE;
		}
	}
	m_Map[12][3] = 0;
}

void CTestMazeSolverDlg::InitMapWithObs()
{
	for (int y = 0; y < 16; ++y)
	{
		for (int x = 0; x < 16; ++x)
		{
			if(m_Map[x][y] != OBSTACLE)
				m_Map[x][y] = NO_OBSTACLE;
		}
	}
	m_Map[12][3] = 0;
}

void CTestMazeSolverDlg::DrawObstacles()
{
	CRect rectClient;
	GetClientRect(&rectClient);

	float cell_dim = rectClient.Height() / 16.0f;

	for (int y = 0; y < 16; ++y)
	{
		for (int x = 0; x < 16; ++x)
		{
			if (m_ObsMap[x][y] == OBSTACLE)
			{
				auto rectTarget = RectF(cell_dim * x + 2.0f, cell_dim * y + 2.0f, cell_dim * (x + 1) - 2.0f, cell_dim * (y + 1) - 2.0f);
				m_BmpTarget->FillRectangle(&rectTarget, m_BmpWhiteBrush.Get());
			}
		}
	}
}

std::string CTestMazeSolverDlg::ToString(const std::wstring& src)
{
	std::string dest = "";
	for (auto ch : src)
		dest += (char)(ch);

	return dest;
}

void CTestMazeSolverDlg::OnBnClickedBtnLoadMaze()
{
	TCHAR szFilter[] = _T("Maze files (*.maze)|*.maze|All files (*.*)|*.*|");

	CFileDialog fdlg(
		TRUE, NULL, NULL,
		OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		szFilter, AfxGetMainWnd());

	if (IDOK == fdlg.DoModal())
	{
		CString existing_file = fdlg.GetPathName();

		using namespace mini;
		csv::ifstream is(ToString((LPCTSTR) existing_file).c_str());
		is.set_delimiter(',', "$$");
		is.enable_trim_quote_on_str(true, '\"');


		int y = 0;
		while (is.read_line())
		{
			try
			{
				int num = 0;
				for (int x = 0; x < 16; ++x)
				{
					is >> num;
					if (num == 1)
						m_ObsMap[x][y] = OBSTACLE;
					else
						m_ObsMap[x][y] = NO_OBSTACLE;
				}

				++y;
			}
			catch (std::runtime_error & e)
			{
				MessageBox(L"Load Maze error!", L"Error", MB_OK | MB_ICONERROR);
			}
		}
		Invalidate(TRUE);
	}
}

void CTestMazeSolverDlg::OnBnClickedBtnSaveMaze()
{
	TCHAR szFilter[] = _T("Maze files (*.maze)|*.maze|All files (*.*)|*.*|");

	CFileDialog fdlg(
		FALSE, CString(".maze"), NULL,
		OFN_EXPLORER | OFN_OVERWRITEPROMPT,
		szFilter, AfxGetMainWnd());

	if (IDOK == fdlg.DoModal())
	{
		CString new_file = fdlg.GetPathName();
		using namespace mini;
		std::wstring file = (LPCTSTR)new_file;

		csv::ofstream os(ToString(file).c_str());
		os.set_delimiter(',', "$$");
		os.enable_surround_quote_on_str(true, '\"');

		for (int y = 0; y < 16; ++y)
		{
			for (int x = 0; x < 16; ++x)
			{
				os << (int)((m_ObsMap[x][y] == OBSTACLE) ? 1 : 0);
			}
			os << NEWLINE;
		}

		os.flush();
		os.close();
	}
}

void CTestMazeSolverDlg::OnBnClickedBtnStart()
{
	m_btnStart.EnableWindow(FALSE);
	m_btnStop.EnableWindow(TRUE);
	m_btnClearAll.EnableWindow(FALSE);

	m_FacingDirection = FacingDirection::North;
	m_Map[12][3] = 0;
	m_PrevCell.x = 0;
	m_PrevCell.y = 15;
	m_CurrCell.x = 0;
	m_CurrCell.y = 15;

	m_PrevObsMode = m_ObsMode;
	m_SolverMode = SolverMode::Solving;
	m_ObsMode = ObsMode::Invalid;

	InitMap();

	m_nWindowTimer = SetTimer(10000, 250, NULL);
}

void CTestMazeSolverDlg::OnBnClickedBtnStop()
{
	m_btnStart.EnableWindow(TRUE);
	m_btnStop.EnableWindow(FALSE);
	m_btnClearAll.EnableWindow(TRUE);

	m_ObsMode = m_PrevObsMode;

	if(m_nWindowTimer>0)
		KillTimer(m_nWindowTimer);
}

void CTestMazeSolverDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == m_nWindowTimer)
	{
		CheckNeighbourObsMap(m_CurrCell.x, m_CurrCell.y);
		InitMapWithObs();
		CalculateMapWeights();
		CPoint nextCell = m_CurrCell;
		DetermineNextCell(m_CurrCell.x, m_CurrCell.y, m_PrevCell.x, m_PrevCell.y, nextCell);
		m_PrevCell = m_CurrCell;
		m_CurrCell = nextCell;

		Invalidate(FALSE);
		if (nextCell.x == 12 && nextCell.y == 3)
		{

			KillTimer(m_nWindowTimer);
			if (m_SolverMode == SolverMode::Solving)
			{
				MessageBox(L"Maze solved! Next is showing the optimized path!", L"Congrats", MB_OK | MB_ICONEXCLAMATION);
				m_FacingDirection = FacingDirection::North;
				m_Map[12][3] = 0;
				m_PrevCell.x = 0;
				m_PrevCell.y = 15;
				m_CurrCell.x = 0;
				m_CurrCell.y = 15;

				m_PrevObsMode = m_ObsMode;
				m_SolverMode = SolverMode::Optimized_Path;
				m_ObsMode = ObsMode::Invalid;

				m_nWindowTimer = SetTimer(10000, 250, NULL);
			}
			else
			{
				m_btnStart.EnableWindow(TRUE);
				m_btnStop.EnableWindow(FALSE);
				m_btnClearAll.EnableWindow(TRUE);
				m_ObsMode = m_PrevObsMode;

				MessageBox(L"The optimized path is completed!", L"Congrats", MB_OK | MB_ICONEXCLAMATION);
			}
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}

void CTestMazeSolverDlg::DetermineNextCell(int x, int y, int prevx, int prevy, CPoint& dest)
{
	int left = GetCellWeight(x, y, prevx, prevy, -1, 0);
	int right = GetCellWeight(x, y, prevx, prevy, 1, 0);
	int up = GetCellWeight(x, y, prevx, prevy, 0, -1);
	int down = GetCellWeight(x, y, prevx, prevy, 0, 1);

	// default behaviour when 2 side cell values are same minimum, always turn right.
	// default behaviour when front and any side cell values are same minimum, always go forward without turning.

	// finding minimum value from the 4 neighbouring cells
	if (left <= up && left <= down && left <= right)
	{
		// default behaviour when left cell shared the same minimum value with another cell
		if (m_FacingDirection == FacingDirection::North && (left == up))
		{
			dest.x += 0;
			dest.y += (-1);
		}
		else if (m_FacingDirection == FacingDirection::South && (left == down))
		{
			dest.x += 0;
			dest.y += 1;
		}
		else if (m_FacingDirection == FacingDirection::East && (left == right))
		{
			dest.x += 1;
			dest.y += 0;
		}
		else if (m_FacingDirection == FacingDirection::North && (left == right))
		{
			m_FacingDirection = FacingDirection::East;
			dest.x += 1;
			dest.y += 0;
		}
		else
		{
			m_FacingDirection = FacingDirection::West;
			dest.x += (-1);
			dest.y += 0;
		}
		return;
	}
	else if (right <= up && right <= down && right <= left)
	{
		// default behaviour when right cell shared the same minimum value with another cell
		if (m_FacingDirection == FacingDirection::North && (right == up))
		{
			dest.x += 0;
			dest.y += (-1);
		}
		else if (m_FacingDirection == FacingDirection::South && (right == down))
		{
			dest.x += 0;
			dest.y += 1;
		}
		else if (m_FacingDirection == FacingDirection::West && (right == left))
		{
			dest.x += (-1);
			dest.y += 0;
		}
		else if (m_FacingDirection == FacingDirection::South && (right == left))
		{
			m_FacingDirection = FacingDirection::West;
			dest.x += (-1);
			dest.y += 0;
		}
		else
		{
			m_FacingDirection = FacingDirection::East;
			dest.x += 1;
			dest.y += 0;
		}
		return;
	}
	else if (up <= left && up <= down && up <= right)
	{
		// default behaviour when up cell shared the same minimum value with another cell
		if (m_FacingDirection == FacingDirection::West && (up == left))
		{
			dest.x += (-1);
			dest.y += 0;
		}
		else if (m_FacingDirection == FacingDirection::East && (up == right))
		{
			dest.x += 1;
			dest.y += 0;
		}
		else if (m_FacingDirection == FacingDirection::South && (up == down))
		{
			dest.x += 0;
			dest.y += 1;
		}
		else if (m_FacingDirection == FacingDirection::East && (up == down))
		{
			m_FacingDirection = FacingDirection::South;
			dest.x += 0;
			dest.y += 1;
		}
		else
		{
			m_FacingDirection = FacingDirection::North;
			dest.x += 0;
			dest.y += (-1);
		}
		return;
	}
	else if (down <= up && down <= left && down <= right)
	{
		// default behaviour when down cell shared the same minimum value with another cell
		if (m_FacingDirection == FacingDirection::West && (down == left))
		{
			dest.x += (-1);
			dest.y += 0;
		}
		else if (m_FacingDirection == FacingDirection::East && (down == right))
		{
			dest.x += 1;
			dest.y += 0;
		}
		else if (m_FacingDirection == FacingDirection::North && (down == up))
		{
			dest.x += 0;
			dest.y += (-1);
		}
		else if (m_FacingDirection == FacingDirection::West && (down == up))
		{
			m_FacingDirection = FacingDirection::North;
			dest.x += 0;
			dest.y += (-1);
		}
		else
		{
			m_FacingDirection = FacingDirection::South;
			dest.x += 0;
			dest.y += 1;
		}
		return;
	}
}

int CTestMazeSolverDlg::GetCellWeight(int x, int y, int prevx, int prevy, int move_x, int move_y)
{
	x += move_x;
	y += move_y;

	if (x < 0 || y < 0 || x>15 || y>15)
		return OBSTACLE;

	return m_Map[x][y];
}

int CTestMazeSolverDlg::GetCellWeight(int x, int y)
{
	if (x < 0 || y < 0 || x>15 || y>15)
		return OBSTACLE;

	return m_Map[x][y];
}

int CTestMazeSolverDlg::CheckObsMap(int x, int y, bool& boundary)
{
	boundary = false;
	if (x < 0 || y < 0 || x>15 || y>15)
	{
		boundary = true;
		return OBSTACLE;
	}

	return m_ObsMap[x][y];
}

void CTestMazeSolverDlg::CheckNeighbourObsMap(int x, int y)
{
	bool boundary = false;
	int num = CheckObsMap(x - 1, y, boundary);
	if (boundary == false && num==OBSTACLE)
	{
		m_Map[x - 1][y] = OBSTACLE;
	}
	num = CheckObsMap(x + 1, y, boundary);
	if (boundary == false && num == OBSTACLE)
	{
		m_Map[x + 1][y] = OBSTACLE;
	}
	num = CheckObsMap(x, y - 1, boundary);
	if (boundary == false && num == OBSTACLE)
	{
		m_Map[x][y - 1] = OBSTACLE;
	}
	num = CheckObsMap(x, y + 1, boundary);
	if (boundary == false && num == OBSTACLE)
	{
		m_Map[x][y + 1] = OBSTACLE;
	}

}

int CTestMazeSolverDlg::GetLowestWeightFromNeighbourCells(int x, int y)
{
	int left = GetCellWeight(x-1, y);
	int right = GetCellWeight(x+1, y);
	int up = GetCellWeight(x, y+1);
	int down = GetCellWeight(x, y-1);

	std::vector<int> v{ left, right, up, down };
	auto it = std::min_element(v.cbegin(), v.cend());
	return *it;
}

void CTestMazeSolverDlg::CalculateMapWeights()
{
	bool changed = false;
	do 
	{
		changed = false;

		for (int y = 0; y < 16; ++y)
		{
			for (int x = 0; x < 16; ++x)
			{
				if (x == 12 && y == 3)
					continue;

				if (m_Map[x][y] == OBSTACLE)
					continue;

				int weight = GetLowestWeightFromNeighbourCells(x, y);
				if (weight == OBSTACLE)
					continue;

				weight += 1;
				if (m_Map[x][y] > weight)
				{
					m_Map[x][y] = weight;
					changed = true;
				}
			}
		}


	} while (changed);
}

void CTestMazeSolverDlg::OnDestroy()
{
	if (m_nWindowTimer > 0)
		KillTimer(m_nWindowTimer);

	CDialogEx::OnDestroy();
}

void CTestMazeSolverDlg::OnBnClickedBtnSaveWeightage()
{
	TCHAR szFilter[] = _T("Weight files (*.wei)|*.wei|All files (*.*)|*.*|");

	CFileDialog fdlg(
		FALSE, CString(".wei"), NULL,
		OFN_EXPLORER | OFN_OVERWRITEPROMPT,
		szFilter, AfxGetMainWnd());

	if (IDOK == fdlg.DoModal())
	{
		CString new_file = fdlg.GetPathName();
		using namespace mini;
		std::wstring file = (LPCTSTR)new_file;

		csv::ofstream os(ToString(file).c_str());
		os.set_delimiter(',', "$$");
		os.enable_surround_quote_on_str(false, '\"');

		for (int y = 0; y < 16; ++y)
		{
			char buf[1000];
			sprintf_s(buf, "%02X|%02X|%02X|%02X|%02X|%02X|%02X|%02X|%02X|%02X|%02X|%02X|%02X|%02X|%02X|%02X",
				m_Map[0][y], m_Map[1][y], m_Map[2][y], m_Map[3][y],
				m_Map[4][y], m_Map[5][y], m_Map[6][y], m_Map[7][y],
				m_Map[8][y], m_Map[9][y], m_Map[10][y], m_Map[11][y],
				m_Map[12][y], m_Map[13][y], m_Map[14][y], m_Map[15][y]);
			std::string str = buf;
			os << str << NEWLINE;
		}

		os.flush();
		os.close();
	}

}

void CTestMazeSolverDlg::OnBnClickedBtnClearAll()
{
	InitMaps();
	Invalidate(FALSE);
}
