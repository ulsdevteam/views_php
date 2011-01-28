<?php
// $Id$

/**
 * A handler to provide a field that is constructed by the administrator using PHP.
 *
 * @ingroup views_field_handlers
 */
class views_php_handler_field extends views_handler_field {

  /**
   * Only let users with use PHP permissions set/modify this handler.
   */
  function php_check_access(&$element) {
    if (!user_access('use PHP for settings')) {
      $element['#disabled'] = TRUE;
      $element['#value'] = $element['#default_value'];
      $element['#description'] .= ' <strong>' . t('Note: you do not have permission to modify this.') . '</strong>';
    }
  }

  /**
   * Implements views_object#option_definition().
   */
  function option_definition() {
    $options = parent::option_definition();
    $options['php_static'] = array('default' => '');
    $options['php_value'] = array('default' => '');
    $options['php_output'] = array('default' => '');
    $options['php_click_sortable'] = array('default' => FALSE);
    return $options;
  }

  /**
   * Implements views_handler#option_definition().
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['php_static'] = array(
      '#type' => 'textarea',
      '#title' => t('Static code'),
      '#default_value' => $this->options['php_static'],
      '#rows' => 5,
      '#description' =>
        t('Code to run right before execution of the view. May be useful to define functions to be re-used in the value and/or output code.') . ' <strong>' . t('Do not include &lt;?php ?&gt; delimiters to enclose PHP code.') . '</strong><br/>' .
        t('Available variables:') . '<br/>' .
        t('$static: can be used to store reusable data per row.')
      ,
    );
    $this->php_check_access($form['php_static']);
    $form['php_value'] = array(
      '#type' => 'textarea',
      '#title' => t('Value code'),
      '#default_value' => $this->options['php_value'],
      '#rows' => 5,
      '#description' =>
        t('Code to construct the value of this field.') . ' <strong>' . t('Do not include &lt;?php ?&gt; delimiters to enclose PHP code.') . '</strong><br/>' .
        t('Available variables:') . '<br/>' .
        t('$data: contains the retrieved record from the database (e.g. $data->nid).') . '<br/>' .
        t('$static: can be used to store reusable data per row.')
      ,
    );
    $this->php_check_access($form['php_value']);
  }

  /**
   * Implements views_handler_field#query().
   */
  function query() {
    $this->field_alias = 'views_php_' . $this->position;
  }

  /**
   * Implements views_handler_field#click_sortable().
   */
  function click_sortable() {
    return !empty($this->options['php_click_sortable']);
  }

  /**
   * Implements views_handler_field#click_sort().
   *
   * @see self::php_post_execute()
   */
  function click_sort($order) {
    $this->php_click_sort_order = $order;
  }

  /**
   *
   * @see views_php_views_pre_execute()
   * @see self::php_post_execute()
   */
  function php_pre_execute() {
    // Ecexute static PHP code.
    if (!empty($this->options['php_static'])) {
      $this->php_static(&$this->php_static_variable);
    }

    // If we're sorting, save the original pager and let Views get all results.
    if (!empty($this->options['php_click_sortable']) && !empty($this->php_click_sort_order)) {
      $this->pager = $this->view->pager; // save the pager
      $this->view->set_items_per_page(0);
    }
  }

  /**
   *
   * @see views_php_views_post_execute()
   */
  function php_post_execute() {
    // Ecexute value PHP code.
    foreach ($this->view->result as $i => $row) {
      $this->view->result[$i]->{$this->field_alias} = $this->php_value($row, $this->php_static_variable);
    }

    // If we're sorting, do the actual sorting then fix the results as per the pager info.
    if (!empty($this->options['php_click_sortable']) && !empty($this->php_click_sort_order)) {
      usort($this->view->result, array($this, 'php_click_sort'));

      // Handle paging logic as in view::execute()
      if (!empty($this->pager['items_per_page'])) {

        if (!empty($this->pager['use_pager']) || !empty($this->view->get_total_rows)) {
          $this->view->total_rows = count($this->view->result) - $this->pager['offset'];
        }

        if (!empty($this->pager['use_pager'])) {
          // dump information about what we already know into the globals
          global $pager_page_array, $pager_total, $pager_total_items;
          // total rows in query
          $pager_total_items[$this->pager['element']] = $this->view->total_rows;
          // total pages
          $pager_total[$this->pager['element']] = ceil($pager_total_items[$this->pager['element']] / $this->pager['items_per_page']);

          // What page was requested
          $pager_page_array = isset($_GET['page']) ? explode(',', $_GET['page']) : array();

          // If the requested page was within range. $this->pager['current_page']
          // defaults to 0 so we don't need to set it in an out-of-range condition.
          if (!empty($pager_page_array[$this->pager['element']])) {
            $page = intval($pager_page_array[$this->pager['element']]);
            if ($page > 0 && $page < $pager_total[$this->pager['element']]) {
              $this->pager['current_page'] = $page;
            }
          }
          $pager_page_array[$this->pager['element']] = $this->pager['current_page'];
        }

        $offset = $this->pager['current_page'] * $this->pager['items_per_page'] + $this->pager['offset'];
        $results = array_slice($this->view->result, $offset, $this->pager['items_per_page']);
        $this->view->result = $results;
      }

      $this->view->pager = $this->pager;
    }
  }

  /**
   *
   */
  function php_static(&$static) {
    ob_start();
    eval($this->options['php_static']);
    return ob_get_clean();
  }

  /**
   *
   */
  function php_value($data, &$static) {
    ob_start();
    print eval('?>'. $this->options['php_value']);
    return ob_get_clean();
  }

  /**
   *
   */
  function php_click_sort($row1, $row2) {

  }
}
